/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "urlencode.h"
#include "util.h"

/* see url(7) for details
 * XXX don't touch the 0 (zero) at the end of either of these arrays! */
/* these are always safe for putting in a URL: */
char url_safe [] = { '-', '_', '.', '!', '~', '*', '\'', '(', ')', '\0' };
/* reserved characters can only be used in certain places: */
char url_reserved [] =
		{ ';', '/', '?', ':', '@', '&', '=', '+', '$', ',', '\0' };

static int is_urlsafe (char c)
{
	int i;
	for (i = 0; url_safe[i]; ++i)
		if (c == url_safe[i])
			return 1;
	return 0;
}

static int is_urlreservedsafe (char c)
{
	int i;
	for (i = 0; url_reserved[i]; ++i)
		if (c == url_reserved[i])
			return 1;
	return 0;
}

static inline void urlencode_common (const char * from, char * to,
		unsigned preserve_reserved)
{
	while (*from) {
		if (isalnum(*from) || is_urlsafe(*from) ||
		  (preserve_reserved && is_urlreservedsafe(*from))) {
			*(to++) = *from;
		} else {
			sprintf(to, "%%%02x", (unsigned char) *from);
			to += 3;
		}
		from++;
	}
}

/* note: + isn't used to substitute as space, since RFCs recommend %20
 * instead.  We're a client, so we don't bother with that.
 */

char * urlencode_dup (const char *str, unsigned preserve_reserved)
{
	char *ret = NULL, *to = NULL;
	
	/* our new string is at _most_ 3 times longer */
	safe_calloc(ret, (strlen(str) * 3) );
	to = ret;
	urlencode_common(str, to, preserve_reserved);
	ret = realloc(ret, strlen(ret) + 1);
	return ret;
}

/* TODO: a function that modifies the string in place */

