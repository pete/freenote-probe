/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
 *	t_ping.c 
 *	Contains the code for testing the icmp module.
gcc -o ping -O3 -Wall -I. -I.. -Ilib/libares p_time.c tests/t_ping.c net-modules/dns.c net-modules/icmp.c lib/libares/*.c
 *
 *	This is an adaptation of the late Mike Muuss's original ping code 
 *	(http://ftp.arl.army.mil/pub/ping.shar), directly from 1983.
 */

/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 * Modified at Uc Berkeley
 *
 * Changed argument to inet_ntoa() to be struct in_addr instead of u_long
 * DFM BRL 1992
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 *
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#include "net-modules/icmp.h"
#include "net-modules/dns.h"
#include "p_time.h"

#define	MAXPACKET	4096	/* max packet size */
#define MAXHOSTNAMELEN	64

extern int errno;

/* static globals. */
static int ident;

int main(int argc, char *argv[])
{
	struct timeval bob;
	int	packetsize = 56, 
		npacks = 4;

	switch(argc) {
		case 4:
			packetsize = atoi(argv[3]);
		case 3:
			npacks = atoi(argv[2]);
		case 2:
			break;
		default:
			fprintf(stderr, "usage: %s host [number_of_packets ["
					"packet_size]]\n", argv[0]);
			exit(1);
	}

	printf("UID:  %d, EUID:  %d\n", getuid(), geteuid());
	/*  EUID will be zero if we are root.  */
	if(geteuid()) {
		fprintf(stderr, "You are not root.  You need to be to ping.\n");
		return 1;
	}

	icmp_test(argv[1], npacks, packetsize, &bob);
	printf("Pinged %s with %d packets of size %d in %d.%06ds.\n", 
			argv[1], npacks, packetsize, bob.tv_sec, bob.tv_usec);
	
	return 0;
}
