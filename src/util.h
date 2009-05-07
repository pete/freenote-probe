/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "io.h"
#include "err.h"

#define safe_malloc(x, y) do { \
	assert((x) == NULL); \
	(x) = malloc(y); \
	if ((x) == NULL) { \
		print_loc(); \
		io_err("Out of memory!\n"); \
		abort(); \
	} \
} while (0)

#define safe_calloc(x, y) do { \
	safe_malloc(x, y); \
	memset((x),0,(y)); \
} while (0)


/** frees and sets a pointer to NULL */
#define safe_free(a) do { \
	if ((a) != NULL ) { \
		free(a); \
		(a) = NULL; \
	} \
} while (0)

#define chomp(a) *(strrchr(a, '\n')) = '\0'

/*  For environments where integers are an even number of bytes, the size of an
    int converted into a string will less than or equal to (2.5 * sizeof(int)).
    This formula started popping up in various places, so I moved it here.
*/
#define INT_TO_S_SIZE ((sizeof(int) / 2) * 5)

/*  To keep #ifdefs to a minimum. */
#ifdef DEBUG
#define DEBUG_MODE_ON 1
#else 
#define DEBUG_MODE_ON 0
#endif

#endif /* _UTIL_H */

