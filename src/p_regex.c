/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "probe.h"
#include "p_regex.h"

#include <string.h>
#include <sys/types.h>
#include <regex.h>

/**
	regex_count
	Given an array of matches and the size of the array, returns the number
	of matches.

	args:
		matches = a regmatch_t array pointer
		max = the number of elements in the array
	returns the number of matches.

	According to the man page, 
		Any  unused  structure elements will contain the value -1.
	This function just counts the elements that are not -1.
*/
int regex_count(regmatch_t matches[], int max)
{
	int 	i = max,
		total = 0;

	while(i--) {
		if(matches[i].rm_so != -1)
			total++;
	}

	return total;
}

/**
	regex_iterate
	
	Iterates over an array of regmatch_t's, calling the provided function
	for each valid element.  Like Ruby's Regexp#match { }.
*/
int regex_iterate(
	regmatch_t matches[], char string[], int max, void *dspace[],
	void (*each_do)(regmatch_t, char *, int, void **))
{
	int i, total = 0;

	for(i = 0; i < max; i++) {
		if(matches[i].rm_so > 0) {	/* matches[i].rm_so != -1 */
			total++;
			each_do(matches[i], string, max, dspace);
		}
	}

	return total;
}
