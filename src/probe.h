/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	probe.h
	General constants and macros for the probe.
	(and one global.)
*/
#ifndef _PROBE_H
#define _PROBE_H

#include "err.h"

#ifndef WIN32
#define bail(x) abort()
#else
#define bail(x) exit(x)
#endif	/* WIN32 not defined */

enum {
	PEXIT_FAILURE = -1,
	PEXIT_NORMAL = 0,
	PEXIT_INTERRUPTED
} onexit;

#define p_exit(x) do {							\
			print_loc();					\
			io_debug("DEBUG:  p_exit.\n");			\
			onexit = x;					\
			exit(x);					\
		} while(0)

#define unless(x) if(!(x))

#define VERSION16BIT		0x100b
#define	PROTOCOL_VERSION	1

#endif /* _PROBE_H defined? */
