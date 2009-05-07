/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	p_time.h	"Probe Time", because time.h is taken.
	Function declarations and data structures for p_time.c,
	which handles time for the probe.
*/

#ifndef _P_TIME_H
#define _P_TIME_H

#include <time.h>
#include <sys/time.h>

/*
	current_time
	Returns a timeval struct for the current time.  
	No arguments.
*/
struct timeval current_time();

/*
	time_difference
	Subtracts two timevals and returns the absolute difference.
	Takes two timevals as arguments.
*/
struct timeval time_difference(struct timeval first, struct timeval second);

/*
	time_sum
	adds two timevals and returns the sum.
*/
struct timeval time_sum(struct timeval first, struct timeval second);

/*
	time_ms
	Given a timeval struct, this function returns the time in 
	ms.
*/
inline int time_ms(struct timeval t);

#endif /* defined _P_TIME_H */
