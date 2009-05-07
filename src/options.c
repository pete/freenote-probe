/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "probe.h"
#include "io.h"
#include "options.h"
#include "util.h"
#include "err.h"

#include "../config.h"
#define DEF_BASE "~/.freenote"
#define DEF_CONFIG (DEF_BASE "/rc")
#define DEF_PIDFILE  (DEF_BASE "/" PACKAGE_NAME ".pid")
#define DISPATCH_SERV	"freenote.petta-tech.com"
#define COLLECT_SERV	"freenote.petta-tech.com"
#define DEFAULT_AGENT ("FreeNoteProbe/" PACKAGE_VERSION)

//FIXME:  the flags have been defined, but permissions do not yet exist
#define O_USER_W	1
#define O_SERV_W	2
#define O_BOTH_W	(O_USER_W|O_SERV_W)

/* options: we don't want to rely on getopt being present, so we have this 
 * simply add new rows to the cli_opts[]
 */
static struct _cli_opts {
	const char * name;     /* with '--' prepended, this is an option */
	const char * def_val;  /* default value of the option */
	char * value;          /* where we store the option as a char ptr */
	unsigned int flags;    /* various flags for this option (unused atm) */
	const char type;       /* 'i' (integer) or 's' (string) */
	const char sname;      /* short option name */
	const char * usage;    /* short description */
} cli_opts [] = {
	/*  User or server specifiable.  */
	
	{ "interval",        "60",           NULL,  O_BOTH_W, 'i', 'i' ,
		"Sleep interval between jobs (Minimum 15)" },
	/*  User-specifiable only.  */
        { "base-dir",        DEF_BASE,       NULL,  O_USER_W, 's', 'b' ,
		"Directory containing the FreeNote config and\nID files" },
	
	{ "daemon",          
#ifndef DEBUG
		"1",
#else
		"0",
#endif
		NULL,  O_USER_W, 'i', 'd' ,
		"Daemonize and run in the background" },
	
	{ "config-file",     DEF_CONFIG,     NULL,  O_USER_W, 's', 'f' ,
		"Configuration file for the FreeNote Probe" },

	{ "pid-file",         DEF_PIDFILE,     NULL,  O_USER_W, 's', 'p' ,
		"Location where the PID file is stored." },
	
	{ "log-method",      "syslog",       NULL,  O_USER_W, 's', 'l' ,
		"Logging method to use" },
	{ "su-user", "nobody",               NULL,  O_USER_W, 's', 'u', 
		"User (username or uid) to attempt to become if possible." },
	{ "verbosity",       
#ifdef DEBUG
		"9",
#else
		"1",
#endif
		NULL,  O_USER_W, 'i', 'v',
		"Messages to report or log.  0 for serious errors\nonly, 1 for "
		"warnings, up to 3 for incredible verbosity." },
	{ "limit-runs",      "0",            NULL,  O_USER_W, 'i', '\0',
		"Exit after a certain number of iterations.\n(0 = unlimited)" },
	{ "version",         "0",            NULL,  O_USER_W, 'i', 'V' ,
		"Show the version number and exit" },
	{ "keepalive",         "1",            NULL,  O_USER_W, 'i', 'k' ,
		"Make sure the probe survives" },
	{ "no-tokens",         "0",            NULL,  O_USER_W, 'i', 'n' ,
		"Force no registration.  (You won't earn tokens.)" },
	{ "setup",             "0",            NULL, O_USER_W, 'i', '\0',
		"Interactive team setup before starting the probe." },
	{ "auto-setup",        NULL,           NULL, O_USER_W, 's', '\0',
		"Non-interactive setup.  Specify 'team_name:pin_code'" },
	{ "help",            "0",            NULL,  O_USER_W, 'i', 'h' ,
		"Show help." },
	/*  Server specifiable only  */
	{ "network-timeout", "120",          NULL,  O_BOTH_W, 'i', '\0', NULL },
	{ "max-page-recursion", "2",         NULL,  O_SERV_W, 'i', '\0', NULL },
	{ "user-agent",      DEFAULT_AGENT,  NULL,  O_SERV_W, 's', '\0', NULL },
	{ "dispatch-server", DISPATCH_SERV,  NULL,  O_SERV_W, 's', '\0', NULL },
	{ "collection-server", COLLECT_SERV, NULL,  O_SERV_W, 's', '\0', NULL },
	/*  Not specifiable.  Like constants.  */
	{ 0 }  /* Terminates the array. */
};

static int opt_is_valid_i(const char *opt, int *idx);

/** read an option from the user */
static int read_opt (int idx, const char * next)
{
	int i;
	switch (cli_opts[idx].type) {
	case 's':
		if (next) {
			safe_free(cli_opts[idx].value);
			cli_opts[idx].value = strdup(next);
			return 1;
		} else {
			print_loc();
			io_err("Error:  Option '%s' requires an argument.\n",
					cli_opts[idx].name);
			p_exit(PEXIT_FAILURE);
		}
		break;
	case 'i':
		if (next) {
			for (i = 0; i < strlen(next); ++i) {
				if (!isdigit(next[i])) {
					safe_free(cli_opts[idx].value);
					cli_opts[idx].value = strdup("1");
					return 0;
				}
			}
			safe_free(cli_opts[idx].value);
			cli_opts[idx].value = strdup(next);
			return 1;
		} else {
			safe_free(cli_opts[idx].value);
			cli_opts[idx].value = strdup("1");
		}
		break;
	default:
		print_loc();
		io_err("Programmer error:  Unknown option type: %c for %s\n",
				cli_opts[idx].type,
				cli_opts[idx].name);
		abort();
		break;
	}
	return 0;
}

/** read a long (dash)(dash)(multi-char) option */
static int get_long_opt(const char * opt, const char * next)
{
	unsigned int i;
	for (i = 0; cli_opts[i].name; ++i)
		if ((!strncmp(opt,cli_opts[i].name,strlen(cli_opts[i].name)))
				&& read_opt(i,next))
			return 1;
	return 0;
}

/** read a short (dash)(single character) option */
static int get_short_opt(const char * opt, const char * next)
{
	unsigned int i;
	for (i = 0; cli_opts[i].name; ++i)
		if ((opt[0] == cli_opts[i].sname) && (read_opt(i,next)))
			return 1;
	return 0;
}

/** read command-line arguments */
static int read_cli_opts(const int argc, char ** argv)
{
	unsigned int i;
	for (i = 1; i < argc; ++i) {
		if (!strncmp(argv[i],"--",2)) {
			char * eq = NULL;
			if ((eq = strchr(argv[i],'=')) != NULL) {
				if (*(eq+1) == '\0') {
					io_err("Option requires argument "
					       "after '='\n");
					return -1;
				}
				*eq = '\0';
				get_long_opt(argv[i]+2,eq+1);
				*eq = '=';
			} else {
				i += get_long_opt(argv[i]+2, (((i+1) < argc)
						  ? argv[(i+1)] : NULL));
			}
		} else if (!strncmp(argv[i],"-",1)) {
			if (*(argv[i]+2) != '\0') {
				char tmp = *(argv[i]+1);
				get_short_opt(&tmp,argv[i]+2);
			} else {
				i += get_short_opt(argv[i]+1,(((i+1) < argc)
				                   ? argv[(i+1)] : NULL));
			}
		} else {
			io_err("Unknown option: %s\n",argv[i]);
			return -1;
		}
	}
	return 0;
}

static int print_option_usage(FILE *s,const unsigned int i)
{
	const char *u;
	unsigned x, z;
	static const unsigned int left_col = 27;
	unsigned pad = left_col - strlen(cli_opts[i].name) - 4;
	char space[81] =	"                                        "
				"                                        ";
	
	if (cli_opts[i].sname != '\0') {
		pad -= 4;
		fprintf(s,"  -%c, --%s", cli_opts[i].sname,
				cli_opts[i].name);
	} else
		fprintf(s,"  --%s", cli_opts[i].name);

	fprintf(s,"%s", space + 80 - pad);
	
	u = cli_opts[i].usage;
	
	for(x = 0; x < strlen(cli_opts[i].usage); ++x) {
		fprintf(s,"%c",u[x]);
		if (u[x] == '\n')
			for (z = 0; z<left_col; ++z)
				fprintf(s," ");
	}
	fprintf(s,"\n");
	return 0;
}

/* 
	dump_options
	This function prints all of the options to stderr.  DEBUG only.
*/
#ifdef DEBUG
/* this is the reference example on how to read options */
void dump_options() 
{
	unsigned int i;
	for (i = 0; cli_opts[i].name; ++i) {
		if (cli_opts[i].type == 's') {
			const char * tmp = read_str_opt(cli_opts[i].name);
			io_debug("%s: %s\n",cli_opts[i].name,tmp);
		} else
			io_debug("%s: %d\n",cli_opts[i].name,
					read_int_opt(cli_opts[i].name));
	}
}
#endif /* DEBUG defined? */

void finish_options()
{
	int i;
	for (i = 0; cli_opts[i].name; ++i)
		safe_free(cli_opts[i].value);
}

int read_int_opt(const char * opt)
{
	static int i = 0;
	int start = i;
	
	for (; cli_opts[i].name; ++i) {
		if ((cli_opts[i].type == 'i') &&
		  !strncmp(opt,cli_opts[i].name,strlen(cli_opts[i].name))) {
			return (cli_opts[i].value ? atoi(cli_opts[i].value)
					: atoi(cli_opts[i].def_val));
		}
	}
	if (start) {
		for (i = 0; i < start; ++i) {
			if ((cli_opts[i].type == 'i') &&
			  !strncmp(opt,cli_opts[i].name,
				   strlen(cli_opts[i].name))) {
				return (cli_opts[i].value ?
						atoi(cli_opts[i].value)
						: atoi(cli_opts[i].def_val));
			}
		}
	}

	io_err("Attempted to access unknown integer option: %s\n",opt);
	abort(); /* programmer error */
	return 0;
}

char * read_file_opt_dup(const char * opt)
{
	char * tmp_file = read_str_opt_dup(opt);
	if (tmp_file == NULL) {
		io_err("%s is NULL!\n", opt);
		p_exit(PEXIT_FAILURE);
	}
	
	if (tmp_file[0] == '~' && tmp_file[1] == '/') {
		char * tmp = NULL, * home = getenv("HOME");
		if (home == NULL) {
			/* put it in current dir: */
			static char * empty_home = "";
			home = empty_home;
		}
		safe_malloc(tmp, strlen(home) + strlen(tmp_file) + 2);
		strcpy(tmp,home);
		strcat(tmp,&(tmp_file[1]));
		safe_free(tmp_file);
		return tmp;
	} 
	return tmp_file;
}

char * read_str_opt_dup(const char * opt)
{
	return strdup(read_str_opt(opt));
}

const char *read_str_opt(const char *opt)
{
	int i;
	if(opt_is_valid_i(opt, &i)) 
		return (cli_opts[i].value?  cli_opts[i].value :
					    cli_opts[i].def_val);
	io_err("Attempted to read unknown string option: %s\n",opt);
	abort(); /* programmer error */
}

/**
 * parse_options
 * parse options passed through the command line
 * warning: this may modify argv!
 */
int parse_options(const int argc, char ** argv)
{
	int r;
	/* do some CLI parsing */
	r = read_cli_opts(argc,argv);
	#ifdef DEBUG
	dump_options();
	#endif

	#ifndef DEBUG	/* Disable sanity checks for developers. */
	if(read_int_opt("interval") < 15) set_opt("interval", "15");
	if(read_int_opt("limit-runs") && read_int_opt("keepalive"))
		set_opt("keepalive","0");
	#endif
	return r;
}

/*
	opt_is_valid_i
	Internal opt_is_valid.  Returns 1 for true, 0 for false.  
	If the idx pointer is not NULL, it puts the index for cli_opts
	in it.
*/
static int opt_is_valid_i(const char *opt, int *idx)
{
	static int i = 0;
	int start = i;
	/* 	Eventually, it would be nice to change this to an iterator 
		like in task.c.  -p */
	for(; cli_opts[i].name; i++) {
		if(!strncmp(opt, cli_opts[i].name, strlen(cli_opts[i].name))) {
			if(idx != NULL)
				*idx = i;
			return 1;
		}
	}
	if (start) {
		for(i = 0; i < start; ++i) {
			if(!strncmp(opt, cli_opts[i].name,
					strlen(cli_opts[i].name))) {
				if(idx != NULL)
					*idx = i;
				return 1;
			}
		}
	}
	return 0;
}

/**
	opt_is_valid
	Passed the name of an option, opt_is_valid returns 0 for invalid 
	option or 1 for valid.
*/
int opt_is_valid(const char *opt)
{
	return opt_is_valid_i(opt, NULL);
}

int set_opt(const char *optname, const char *optval)
{
	int i;
	if(opt_is_valid_i(optname, &i)) {
		safe_free(cli_opts[i].value);
		cli_opts[i].value = strdup(optval);
		return 0;
	}

	io_err("Attempted to edit unknown string option: %s\n", optname);
	abort(); /* programmer error */
}

void options_usage(FILE *s)
{
	unsigned int i;
	for (i = 0; cli_opts[i].usage; ++i)
		print_option_usage(s,i);
}

int is_default_opt(const char *opt)
{
	int i;
	if(opt_is_valid_i(opt, &i)) 
		return (cli_opts[i].value?  0 :
					    1 );
	io_err("Attempted to read unknown option: %s\n",opt);
	abort(); /* programmer error */
}
	

