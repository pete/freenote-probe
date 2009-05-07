#!/usr/bin/env ruby
require 'socket'

def dhms(x)
	s = x%60
	x /= 60
	m = x%60
	x /= 60
	h = x%24
	x /= 24
	[x, h, m, s]
end

def hline(l)
	case l
		when /^Uptime\W*(\d+)s/
			tmp = dhms $1.to_i
			printf "Uptime:  %d day#{"s" if tmp[0] != 1}, " + 
				"%d:%02d:%02d\n", tmp[0], tmp[1], tmp[2], tmp[3]
			print "Time now:  " + `date`
		else
			puts l
	end
end

u = UDPSocket.new
u.bind 'localhost', 14597

x = ""
s = ""
sars = []

puts "Starting the FreeNote Probe Monitor..."
while true 
	s += (u.recvfrom 2048)[0]; 
	if(/\n/.match s)
		sars = s.split("\n")
		s = s[(s.rindex("\n") + 1)...s.length]
		sars.pop if s != ""
		while (x = sars.shift)
			hline x
		end
	end
end
