/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _P_REGEX_H
#define _P_REGEX_H

#include "probe.h"
#include <sys/types.h>
#include <regex.h>

int regex_count(regmatch_t matches[], int max);
/*
int regex_iterate( regmatch_t matches[], char string[], int max, 
	void (*each_do)(regmatch_t, char *, int));
*/
int regex_iterate( regmatch_t matches[], char string[], int max, void *dspace[],
	void (*each_do)(regmatch_t, char *, int, void **));

#endif /* _P_REGEX_H not defined? */
