/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	The probecast subsystem provides info to external programs running
	on the same host.
*/

#ifndef _PROBECAST_H
#define _PROBECAST_H

#define PROBECAST_PORT htons(14597)

/**
	probecast_init
	Initializes the probecaster.  Returns -1 for failure, but there
	is usually no need to catch it, as this is an optional feature.
*/
int probecast_init();

/**
	probecast
	Spew information out the probecast socket for use by external
	programs.  Fun for the whole family!
	buf = data to pass
	size = number of bytes to send.
	Returns 0 for success, -1 for failure.
*/
int probecast(char *buf, int size);

/**
	probecasts
	As above, but assumes a null-terminated string, so it requires one
	argument less.
*/
int probecasts(char *buf);

/**
	Shutdown function for the probecaster.  Call for cleanup.  Returns
	-1 for failure, 0 for success.
*/
int probecast_end();

#endif
