/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _NET_MODULE__ICMP_H
#define _NET_MODULE__ICMP_H

/*	
	icmp.h 
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>

#ifdef WIN32
# include <winsock.h>
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netinet/in_systm.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <netdb.h>
#endif /* WIN32 */

#include "net-modules/dns.h"
#include "p_time.h"

#define	MAXPACKET	4096	/* max packet size */
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN  64
#endif /* !defined(MAXHOSTNAMELEN) */

extern int errno;

/*  public function.  */
int icmp_ping(char *hostname, int npackets, int packetsize, struct timeval *t);

#endif /* _NET_MODULE__ICMP_H */

