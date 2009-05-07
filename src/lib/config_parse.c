/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>

#include "io.h"
#include "options.h"
#include "lib/config_parse.h"

extern int errno;

static int parse_config(FILE *stream);
static int parse_line(char *line);

static int parsed_yet = 0;	/*  So that SIGHUP behaves properly.  */

/*
	These are all kinda complicated, long, and ugly.  (I'm no good parsing
	text in C...)  I'd like to come back to this to simplify it.
*/

/**
	read_config
	Reads and parses the specified config file, and sets the appropriate 
	options.
	Returns -1 on error, 0 on success.
*/
int read_config(const char *filename)
{
	FILE *cf;
	int r;

	cf = fopen(filename, "r");
	if(cf == NULL) {
		io_debug("Error opening config file '%s':  %s\n"
			"Skipping config.\n", filename, strerror(errno));
		return 0;
	}

	r = parse_config(cf);
	fclose(cf);
	parsed_yet = 1;

	if(r == -1)
		io_err("Trouble parsing %s.\n", filename);

	return r;
}

/*
	parse_config
	Reads and config file, removing comments and checking for errors.  Gets
	parse_line() to parse the individual lines.
	Returns 0 for success, -1 for error.
*/
static int parse_config(FILE *stream)
{
	int	lineno;
	char 	line[MAXPATHLEN],	/* should be long enough */
		*idx;
	
	for(lineno = 1; fgets(line, MAXPATHLEN, stream) != NULL; lineno++) {
		idx = strchr(line, '#');
		if(idx != NULL)	
			*idx = '\0';
		if(opt_is_valid(line)) {
			if(parse_line(line) == -1) {
				io_err("Could not parse line %d\n", lineno);
				return -1;
			}
		} else {
			io_err("Unrecognized option on line %d.\n", lineno);
			return -1;
		}
	}
	return 0;
}


/*
	parse_line
	Parses a line from the config file.  Lines must be of the form
	"option-name[whitespace]value[\n]"
	We're kinda restrictive for now, but the options are so simple that
	we shouldn't need anything complex for now.
	Returns -1 on error, 0 otherwise.
*/
static int parse_line(char *line)
{
	char	*opt = line, 
		*val,
		*p;

	p = strchr(line, '\n');	/* Strip the trailing newline. */
	if(p != NULL)
		*p = '\0';

	/* Seperate the value from the option. */
	val = opt;
	while(!isspace(*val)) {
		val++;
		if(*val == '\0') {
			io_err("Missing value for '%s'!'\n", opt);
			return -1;
		}
	}
	
	*val = '\0';
	val++;

	while(isspace(*val)) {
		val++;
		if(*val == '\0') {    
                        io_err("Missing value for '%s'!'\n", opt);
                        return -1;
                }
	}

	if(is_default_opt(opt) && !parsed_yet) 
		return set_opt(opt, val);
	return 0;
}

