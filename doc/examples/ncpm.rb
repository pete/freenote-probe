#!/usr/bin/env ruby
require 'socket'
require 'ncurses.rb'
include Ncurses::Namespace

$probe_id = 0
$uptime = ""
$jobs_complete = 0
$networks = Hash.new(nil)

def dhms(x)
	s = x%60
	x /= 60
	m = x%60
	x /= 60
	h = x%24
	x /= 24
	[x, h, m, s]
end

def formatup(d)
	tmp = dhms d.to_i
	sprintf "%d day#{"s" if tmp[0] != 1}, %d:%02d:%02d", 
		tmp[0], tmp[1], tmp[2], tmp[3]
end

def addnetdata(data)
	info = data.split ','
	ip, size, time = info[3], info[4].to_i, info[9].to_i
	ip = ip.split('.')[0, 3].join('.') + ".*"
	if $networks[ip].nil?
		owner = 
		       ((`whois 4.2.2.1`.grep(/^NetName/)[0]).split /\W+/, 2)[1]
		owner.chomp!
		$networks[ip] = Hash["jobs" => 1, "time" => time, 
			"size" => size, "owner" => owner]
	else
		$networks[ip]["jobs"] += 1
		$networks[ip]["time"] += time
		$networks[ip]["size"] += size
	end
	$networks[ip]
end

def hline(l)
	case l
		when /^Uptime\W*(\d+)s/
			$uptime = formatup($1)
			nil
		when /^Completed:  ((.*,).+)+/
			addnetdata $1
			nil
		when /^Probeid:  (\d+)/
			$probe_id = $1
		when /^Status:  (.+)/
			$status = $1
		when /^Jobs Complete:  (\d+)/
			$jobs_complete = $1
			nil
		else
			l
	end
end

#  int main(void)
puts "Starting the FreeNote Probe Monitor..."

u = UDPSocket.new
u.bind 'localhost', 14597

x = ""
s = ""
sars = []

initscr ; at_exit { endwin }

["INT", "TERM"].each { |sig|
	(lambda { |s|
		trap(s) {
			endwin
			puts "Caught Signal#{s}!\nExiting!"
			exit
		}
	}).call(sig)
}

noecho
halfdelay 1
curs_set 0

while true 	#  Or until we get a break or a SIG{INT,TERM,KILL,...}
	s += (u.recvfrom 2048)[0]; 
	if(/\n/.match s)
		#  The messages from the probe are line-delimited.  So we
		#  read until we have at least one complete line, then we
		#  run hline on each complete line.
		sars = s.split("\n")
		s = s[(s.rindex("\n") + 1)...s.length]
		sars.pop if s != ""
		while (x = sars.shift)
			printw "<raw>#{x}</raw>\n" if ARGV.include? "raw"
			x = hline(x)
			printw(x + "\n") unless x.nil?
		end
	end

	move 0, 0
	printw "The FreeNote Probe Monitor\t\thttp://freenote.petta-tech.com\n"
	printw "Uptime:  #{$uptime}\t\t#{`date`.chomp}\n"
	printw 	"Monitoring Probe ##{$probe_id}\t\t\t\t" + 
		"Jobs complete:  #{$jobs_complete}\n"
	printw("=" * 75 + "\n")
	printw(sprintf("%15s %24s\t%s\t%s\n",  
		"Network", "Owner", "Jobs", "Average Speed"))
	$networks.each_key { |ip|
		printw(sprintf("%15s %24s\t%s\t%.2f kbps\n", ip, 
		    $networks[ip]["owner"], $networks[ip]["jobs"],
		    $networks[ip]["size"].to_f / $networks[ip]["time"].to_f))
	}
	printw("=" * 75 + "\n")
	refresh
end
