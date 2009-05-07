/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/**
 * functions exported for parsing options (command-line and config-file)
 */

#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <stdio.h>

/** returns the string pointer a for option @opt.
 *  WARNING: this uses strdup(3) internally, so remember to free the pointer
 *  this function returns
 */
char * read_str_opt_dup(const char * opt);

/* As above, but returns a const char * that does not need to be free()d.  */
const char *read_str_opt(const char *opt);

/** like read_str_opt_dup, but expands "~/"-prefixed files */
char * read_file_opt_dup(const char * opt);

/** returns the integer value for an integer option, @opt */
int read_int_opt(const char * opt);

/* parse options (both cli and config file */
int parse_options(const int argc, char ** argv);

/* checks to see if an option is valid. */
int opt_is_valid(const char *opt);

/* sets an option */
int set_opt(const char *optname, const char *optval);

/* displays help on the options, prints to file stream s */
void options_usage(FILE *s);

/* checks if an option is set to the default or not */
int is_default_opt(const char *opt);

/* frees up memory used for options */
void finish_options();

/* global constants */

#endif /* _OPTIONS_H */


