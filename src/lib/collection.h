/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _LIB__COLLECTION_H
#define _LIB__COLLECTION_H

#include <sys/time.h>
#include <time.h>

typedef struct _report {
	struct timeval elapsed;
} report;

int send_results(const char *server);

#endif /* _LIB__COLLECTION_H */


