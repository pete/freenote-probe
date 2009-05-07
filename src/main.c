/*
  FreeNote Probe
  Copyright (C) 2005 Petta Technology, Inc.  <freenote@petta-tech.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; version 2
  of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public
  License along with this program; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.

  In addition, as a special exception, Petta Technology, Inc.
  gives permission to link the code of portions of this program
  with the OpenSSL library under certain conditions as described
  in each individual source file, and distribute linked
  combinations including the two.
  
  You must obey the GNU General Public License in all respects for
  all of the code used other than OpenSSL.  If you modify file(s)
  with this exception, you may extend this exception to your
  version of the file(s), but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.  If you delete this exception statement from all
  source files in the program, then also delete it here.
  
  For DNS resolution, the FreeNote Probe uses a modified version
  of the Ares asynchronous resolver library.  Our modified version 
  of libares is redistributed under the original terms of the MIT license 
  (free software).  See the comments at the top of src/lib/libares/ares.h for 
  the full license text.
  
  For ICMP/ping functionality, we adapted the public domain code
  of Mike Muuss, found at http://ftp.arl.mil/~mike/ .
*/
/*
 * The FreeNote Probe Authors:
 * Pete Elmore <pete@petta-tech.com>
 * Phil Lalone <phil@petta-tech.com>
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "../config.h"
#include "sys/compat.h"
#include "probe.h"
#include "debug.h"
#include "io.h"
#include "options.h"
#include "work.h"
#include "util.h"
#include "lib/config_parse.h"
#include "lib/dispatch.h"
#include "lib/collection.h"
#include "lib/task.h"
#include "lib/fileman.h"
#include "lib/probecast.h"
#include "crypt/dsa/pdsa.h"
#include "err.h"
#include "net-modules/dns.h"
#include "setup.h"

void usage(FILE *s, char *name);
int subsystems_init();
void subsystems_finish();
int mainloop();
int mainiter(int *retval);
void maybe_daemonize();
void exit_message(int rcode);
void print_version();

/* Sighandlers */
static void catch_cont(int signum);
static void catch_hup(int signum);
static void catch_int(int signum);
static void catch_term(int signum);
static void reg_sighandlers();

static pid_t child_pid = -1;
/* Just for fun. */
static time_t	probe_uptime;
static int	jobs_completed;
/* Static globals are initialized to 0 implicitly. */

int main (int argc, char ** argv)
{
	char	*rc;

	io_info("Starting the FreeNote Probe version %s.\n", VERSION);
	
	if(parse_options(argc, argv) < 0) {
		usage(stderr, argv[0]);
		p_exit(PEXIT_FAILURE);
	} 

	if(read_int_opt("help") == 1) {
		usage(stdout, argv[0]);
		return 0;
	}

	if(read_int_opt("version") == 1) {
		print_version();
		return 0;
	}

	rc = read_file_opt_dup("config-file");
	try(read_config(rc));
	safe_free(rc);

	try(maybe_setup());
	maybe_daemonize();
	try(subsystems_init());
	io_debug("DEBUG OUTPUT ENABLED!\n");

	onexit = mainloop();	/* The part that matters. */
	fflush(NULL);   /* flush any messages we have left */

	return 0;
}

void print_version()
{
	printf("The FreeNote Probe, version " VERSION "\n");
}

/*
	usage
	Prints the usage for the probe.  Takes a stream (s), which should be
	stderr for incorrect usage, or stdout if the user explicitly passed
	'-h' or '--help'.  'name' is the name of the program (argv[0]).
*/
void usage(FILE *s, char *name)
{
	print_version();
	fprintf(s, "Usage:  %s [options]\n", name);
	options_usage(s);
}

/*
	subsystems_init
	Starts up the various IO, network, job stack, etc. subsystems.
	If something goes wrong, we return -1.  Otherwise, it's 0.
*/
int subsystems_init()
{
	/*  Some of these cannot fail right now => no error checking. */
	io_init();
	try(fm_init_base());	/* ~/.freenote */
	tasks_init();		/* Task array */
	populate_handlers();	/* Task handlers for work.c */
	dns_init();		/* Sets up the DNS cache. */
	try(dsa_init());
	probecast_init();

	atexit(subsystems_finish);

	probe_uptime = time(NULL);

	return 0;
}

/*
	subsystems_finish
	Do some final cleanup before exit.
*/
void subsystems_finish()
{
	dns_cache_flush();
	dsa_close();
	exit_message(onexit);	// onexit is a global defined in probe.h
	probecast_end();
	probe_uptime = time(NULL) - probe_uptime;
	io_info("Probe uptime: %d minutes.\nJobs completed:  %d.\n",
		probe_uptime / 60, jobs_completed);
	io_finish();
	finish_options();
}

/*
	mainloop
	Runs the main loop until we have a reason to exit.
	Returns 0 for success, -1 for problems;
*/
int mainloop()
{
	int 	r,
		limit_runs;
	char message[1024];

	limit_runs = read_int_opt("limit-runs");
	while(!mainiter(&r)) {
		if(r == 0) {
			io_info("Completed an iteration.  Sleeping for"
				" %d second(s).\n", read_int_opt("interval"));
			
			jobs_completed++;
			sprintf(message, "Uptime:  %lds\nJobs Complete:  %d\n"
				"Status:  Sleeping.\nPID:  %d\n",
				time(NULL) - probe_uptime, jobs_completed, 
				getpid());
			probecasts(message);
			fflush(NULL);
			sleep(read_int_opt("interval"));
		}
		if(limit_runs && !(jobs_completed < limit_runs)) {
			break;
		}
	}
	return (r == 0?	 0 :
			-1);
}

/*
	mainiter
	Runs through an iteration of the main loop.  
	Returns nonzero if it's time to exit.  In this case, it gives the 
	return value in a 
*/
int mainiter(int *retval)
{
	*retval = -1;
	if(get_jobs(read_str_opt("dispatch-server")) == -1) {
		print_loc();
		io_err(	"Couldn't get jobs from the dispatch server!\n"
			"Sleeping for %d minutes before retrying.\n", 
			(read_int_opt("interval") * 10) / 60);
		fflush(NULL);
		probecasts("Error!  Couldn't get jobs!\n");
		sleep(read_int_opt("interval") * 10);
		return 0;
	}
	do_or_die(	process_tasks(),
			"Fatal error encountered when processing tasks!\n");
	do_or_die(	send_results(read_str_opt("collection-server")),
			"Fatal error trying to send results to the collection"
			" server!\n");

	dns_cache_flush();	/* So future jobs' data is right. */

	*retval = 0;
	return 0;
}

/*
	maybe_daemonize
	Checks options, and sets appropriate logging.  Maybe Daemonizes.
*/
void maybe_daemonize()
{
	if (read_int_opt("daemon")) {
		daemonize();
	} else {
		if (is_default_opt("log-method")) 
			set_opt("log-method","stdout");
	}
	io_info("Log method:  %s\n", read_str_opt("log-method"));

	reg_sighandlers();
	fm_create_pidfile(); /* this must be called _after_ daemonize */
	while(read_int_opt("keepalive") && ((child_pid = fork())>0)) {
		wait(NULL);
		io_info("Probe with pid %d died.  "
			"Sleeping then forking a new one.\n", child_pid);
		probecasts("Error:  Dead probe!  Re-starting...\n");
		sleep(read_int_opt("interval"));
	}
	if(read_int_opt("keepalive"))  {
		if (child_pid != 0)
			fm_remove_pidfile();
	} else
		atexit(fm_remove_pidfile);
}

void exit_message(int rcode)
{
	probecasts("Exiting:  ");
	switch(rcode) {
		case PEXIT_NORMAL:
			probecasts("normally.\n");
			io_info("Bye.\n");
			break;
		case PEXIT_FAILURE:
			probecasts("errors!\n");
			io_err("Encountered unrecoverable errors.  Exiting.\n");
			break;
		case PEXIT_INTERRUPTED:
			probecasts("interrupted!\n");
			break;
		default:
			probecasts("unknown status!\n");
			break;
	}
}

static void catch_cont(int signum) 
{
	signal(SIGCONT, catch_cont);
	io_err("Stopping and continuing the probe skews results!\nExiting.\n");
	p_exit(PEXIT_FAILURE);
}

static void catch_hup(int signum)
{
	signal(SIGHUP, catch_hup);
	io_info("Caught SIGHUP.  Re-reading config file...\n");
	{
		char * conffile = read_file_opt_dup("config-file");
		read_config(conffile);
		safe_free(conffile);
	}
	fflush(NULL);
}

static void catch_int(int signum)
{
	signal(SIGINT, catch_int);
	io_info("Caught interrupt! Exiting.\n");
	if (child_pid > 0)
		kill(child_pid, signum);
	p_exit(PEXIT_INTERRUPTED);
}

static void catch_term(int signum)
{
	signal(SIGTERM, catch_term);
	io_warning("Caught TERM.  Cleaning up...\n");
	if (child_pid > 0)
		kill(child_pid, signum);
	p_exit(PEXIT_FAILURE);
}

/* Initialize signal handlers */
static void reg_sighandlers()
{
	signal(SIGCONT, catch_cont);
	signal(SIGHUP, catch_hup);
	signal(SIGINT, catch_int);
	signal(SIGTERM, catch_term);
}
