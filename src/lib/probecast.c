/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "lib/probecast.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h> 
#include "err.h"

static struct sockaddr localhost;
static int sockfd = 0;

/**
	probecast_init
	Initializes the probecaster.  Returns -1 for failure, but there
	is usually no need to catch it, as this is an optional feature.
*/
int probecast_init()
{
	struct sockaddr_in lh = {
		AF_INET,
		PROBECAST_PORT,
		{INADDR_ANY},
		{0,0,0,0,0,0,0,0}
		};
	memcpy(&localhost, &lh, sizeof(struct sockaddr));
	try(sockfd = socket(AF_INET, SOCK_DGRAM, 0));
	return 0;
}

/**
	probecast
	Spew information out the probecast socket for use by external
	programs.  Fun for the whole family!
	buf = data to pass
	size = number of bytes to send.
	Returns 0 for success, -1 for failure.
*/
int probecast(char *buf, int size)
{       
	try(sockfd);
	return (sendto(sockfd, buf, size, 0, (struct sockaddr *)&localhost,
		sizeof(struct sockaddr)) == size) - 1;
}

/**
	probecasts
	As above, but assumes a null-terminated string, so it requires one
	argument less.
*/
int probecasts(char *str)
{
	return probecast(str, strlen(str));
}


/**
	Shutdown function for the probecaster.  Call for cleanup.  Returns
	-1 for failure, 0 for success.
*/
int probecast_end()
{
	try(sockfd);
	close(sockfd);
	return 0;
}
