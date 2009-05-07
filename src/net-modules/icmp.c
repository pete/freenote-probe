#ifndef CYGWIN32 
#include "net-modules/icmp.h"

/*
 *	icmp.c 
 *	Contains the code for running icmp tasks.  
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>

# include <arpa/inet.h>
# include <sys/socket.h>

#include "inaddr_none.h"
#include "net-modules/icmp.h"
#include "net-modules/dns.h"
#include "p_time.h"
#include "io.h"

extern int errno;

/*  Static functions.  */
static int in_cksum(struct icmp *addr, int len);
static int pr_pack(char *buf, int cc, struct sockaddr_in *from);
static void pinger(int datalen, struct sockaddr_in whereto, int sockfd, 
	int seq);

/* static globals. */
static int ident;

/*
	icmp_ping
	Given a host-name, the number of packets to send, the packet size 
	(which should usually be 56), and a pointer to a timeval struct to
	put completion time in (or NULL if you don't care), it pings that host
	and returns the number of packets that made it back.
 */
int icmp_ping(char *hostname, int npackets, int packetsize, struct timeval *t)
{
	struct in_addr 	hostip;
	struct sockaddr_in 	from, 
				whereto;	/* who to ping */
	struct protoent *proto;
	int 	fromlen = sizeof(from),		/* apparently unused. */
		cc, 		/* mysterious */
		s,		/* the socket file descriptor */
		i;		/* the iterator */
	u_char	packet[MAXPACKET];
	char 	hnamebuf[MAXHOSTNAMELEN];

	memset((char *) &whereto, 0, sizeof(struct sockaddr));
	whereto.sin_family = AF_INET;
	whereto.sin_addr.s_addr = inet_addr(hostname);
	if (whereto.sin_addr.s_addr != (unsigned) -1) {
		strcpy(hnamebuf, hostname);
	} else {
		hostip = resolve(hostname, NULL, NULL);
		if (hostip.s_addr != INADDR_NONE) {
			whereto.sin_family = AF_INET;
			whereto.sin_addr = hostip;
		} else {
			io_err("unknown host: %s\n", hostname);
			return -1;
		}
	}

	ident = getpid() & 0xFFFF;

	if ((proto = getprotobyname("icmp")) == NULL) {
		io_err("icmp: unknown protocol\n");
		return -1;
	}

	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("ping: socket");
		return -1;
	}

	setlinebuf(stdout);

	if(t != NULL)
		*t = current_time();
	for(i = 0; i < npackets; i++) {
		pinger(packetsize, whereto, s, i);
		cc = recvfrom(s, packet, sizeof(packet), 0,
		      (struct sockaddr *) &from, &fromlen);
		pr_pack(packet, cc, &from);
	}
	if(t != NULL)
		*t = time_difference(current_time(), *t);
	return 0;
}

/*
 * 			P I N G E R
 * 
 * Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
static void pinger(int datalen, struct sockaddr_in whereto, int sockfd, int seq)
{
	static u_char outpack[MAXPACKET];
	struct icmp *icp = (struct icmp *) outpack;
	int i, cc;
	struct timeval *tp = (struct timeval *) &outpack[8];
	u_char *datap = &outpack[8 + sizeof(struct timeval)];

	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = seq;
	icp->icmp_id = ident;	/* ID */

	cc = datalen + 8;	/* skips ICMP portion */

	*tp = current_time();

	for (i = 8; i < datalen; i++)	/* skip 8 for time */
		*datap++ = i;

	/* Compute ICMP checksum here */
	icp->icmp_cksum = in_cksum(icp, cc);

	/* cc = sendto(s, msg, len, flags, to, tolen) */
	i = sendto(sockfd, outpack, cc, 0, (struct sockaddr *)&whereto, 
		sizeof(struct sockaddr));

	if (i < 0 || i != cc) {
		if (i < 0)
			perror("sendto");
		io_debug("ping: wrote %d chars, ret=%d\n",
		       cc, i);
		fflush(stdout);
	}
}

/*
 * 			P R _ T Y P E
 *
 * Convert an ICMP "type" field to a printable string.
 */
const char *pr_type(int t)
{
	static const char *ttab[] = {
		"Echo Reply",
		"ICMP 1",
		"ICMP 2",
		"Dest Unreachable",
		"Source Quench",
		"Redirect",
		"ICMP 6",
		"ICMP 7",
		"Echo",
		"ICMP 9",
		"ICMP 10",
		"Time Exceeded",
		"Parameter Problem",
		"Timestamp",
		"Timestamp Reply",
		"Info Request",
		"Info Reply"
	};

	if (t < 0 || t > 16)
		return ("OUT-OF-RANGE");

	return (ttab[t]);
}

/*
	pr_pack
	Returns 1 if we got our packet, 0 otherwise.
   This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
int pr_pack(char *buf, int cc, struct sockaddr_in *from)
{
	struct ip *ip;
	register struct icmp *icp;
	struct timeval tv;
	struct timeval *tp;
	int hlen, triptime;

	gettimeofday(&tv, NULL);

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN) {
		return 0; /* packet too short. */
	}
	cc -= hlen;
	icp = (struct icmp *) (buf + hlen);

	if (icp->icmp_id != ident)
		return 0; /* intended for a different process. */

	tp = (struct timeval *) &icp->icmp_data[0];
	tv = time_difference(tv, *tp);
	triptime = tv.tv_sec * 1000 + (tv.tv_usec / 1000);
	return 1;
}


/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
int in_cksum(struct icmp *addr, int len)
{
	int nleft = len;
	u_short *w = (ushort *) addr;
	u_short answer;
	int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		u_short u = 0;

		*(u_char *) (&u) = *(u_char *) w;
		sum += u;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);	/* add carry */
	answer = ~sum;		/* truncate to 16 bits */
	return (answer);
}

#endif /* !CYGWIN32 */

