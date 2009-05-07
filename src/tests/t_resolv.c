/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	tests the resolver.  
	Do this:
gcc -O3 -Wall -I. -I.. -I../lib/libares ../net-modules/dns.c ../lib/libares/*.c ../p_time.c t_resolv.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "net-modules/dns.h"
#include "p_time.h"

void usage(FILE *stream);
void res(char *name, char *ns);

int main(int argc, char **argv)
{
	int i;
	char *ns = NULL;

	if(argc < 2) {
		usage(stderr);
		exit(1);
	}
	dns_init();
	ns = argv[1];

	for(i = 2; i < argc; i++) {
		res(argv[i], ns);
	}

	return 0;
}

void res(char *name, char *ns)
{
	struct timeval time;
	struct in_addr address;

	address = resolve(name, ns, &time);
	printf("%s = %s (query took %d.%06d seconds from %s.)\n",
		name,
		inet_ntoa(address),
		time.tv_sec,
		time.tv_usec,
		ns
		);

	address = resolve_cached(name, ns, &time);
	printf("(%s) %d.%06ds to resolve from cache\n", 
		inet_ntoa(address), time.tv_sec, time.tv_usec);
	address = resolve_cached(name, ns, &time);
	printf("(%s) %d.%06ds to resolve from cache\n", 
		inet_ntoa(address), time.tv_sec, time.tv_usec);
	address = resolve_cached(name, ns, &time);
	printf("(%s) %d.%06ds to resolve from cache\n", 
		inet_ntoa(address), time.tv_sec, time.tv_usec);
}

	

void usage(FILE *stream)
{
	fprintf(stream, "Supply two arguments:  a nameserver to use followed "
			"by at least one hostname\nto resolve.\n");
}
