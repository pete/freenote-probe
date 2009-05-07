/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _PROBE_DNS_H
#define _PROBE_DNS_H

#include <sys/time.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/libares/ares.h"
#include "lib/libares/ares_dns.h"

#include <time.h>

#include "inaddr_none.h"

#define MAX_DNS_CACHE_SIZE 32	/* Seems reasonable. */

/*  DNS Cache Entry data structure */
typedef struct {
	char *name; 		/* e.g., www.hughesmissiles.com  */
	struct in_addr addr;	/* e.g., inet_addr("66.150.188.102") */
} dns_centry;

void dns_init();
void dns_cache_flush();

struct in_addr resolve(const char *name, const char *nameserver_ip, 
	struct timeval *time);
struct in_addr resolve_cached(const char *name, const char *ns, 
	struct timeval *time);

#endif /* defined? _PROBE_DNS_H */


