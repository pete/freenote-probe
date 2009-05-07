/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "urldecode.h"
#include "util.h"

static inline void urldecode_common (const char * from, char * to)
{
	int tmp;
	while (*from) {
		if (*from == '%') {
			++from;
			sscanf(from, "%2x", &tmp);
			++from;
			*(to++) = (char)tmp;
		} else {
			*(to++) = *from;
		}
		from++;
	}
}

/* note: + isn't used to substitute as space, since RFCs recommend %20
 * instead.  We're a client, so we don't bother with that.
 */

char * urldecode_dup (const char *str)
{
	char *ret = NULL, *to = NULL;

	assert(str != NULL);
	
	safe_calloc(ret, strlen(str)+1 );
	to = ret;
	urldecode_common(str, to);
	ret = realloc(ret, strlen(ret) + 1);

	return ret;
}

/* TODO: a function that modifies the string in place */
