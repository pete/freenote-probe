/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#define DEBUG 1
#define DEBUG_LEVEL 0x00000001 

#define dbg(a,...) do { \
	if (a & DEBUG_LEVEL) \
		fprintf(stderr, __VA_ARGS__); \
} while (0)


#endif /* _DEBUG_H */

