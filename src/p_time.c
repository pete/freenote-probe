/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */


#include <stdio.h>

#include "p_time.h"
#include "io.h"

/**
	current_time
	No arguments.
	Returns a timeval struct with the current time.
*/
struct timeval current_time()
{
	struct timeval time;
	int status;

	/*  	Since this is Unix, it's easy.
		This will get uglier when we have to do #ifdef WIN32.

		By the way, a NULL timezone works fine under Linux, but I don't
		know if it might cause problems on other platforms.
	*/
	status = gettimeofday(&time, NULL);
	
	if(status == -1) {
		io_err("current_time:  Could not gettimeofday!\n");
	}

	return time;
}

/**
	time_difference
	Subtracts two time values, and returns the difference.
	args:
		two struct timevals.
	returns:
		The absolute difference.

	If you pass strange values, it may return strange values.
*/
struct timeval time_difference(struct timeval first, struct timeval second)
{
	struct timeval result;

	if(first.tv_sec > second.tv_sec || 
	    (first.tv_sec == second.tv_sec && 
	    first.tv_usec > second.tv_usec)) {
		result.tv_usec = first.tv_usec - second.tv_usec;
		result.tv_sec = first.tv_sec - second.tv_sec;
	} else {
		result.tv_usec = second.tv_usec - first.tv_usec;
		result.tv_sec = second.tv_sec - first.tv_sec;
	}
	
	/* For valid inputs, usec cannot be less than -999,999. */
	if(result.tv_usec < 0) {
		result.tv_sec--;
		result.tv_usec += 1000000;
	}

	return result;
}

/**
	time_sum
	Adds two timevals, and returns the sum.
*/
struct timeval time_sum(struct timeval first, struct timeval second)
{
	struct timeval result;

	result.tv_sec = first.tv_sec + second.tv_sec;
	result.tv_usec = first.tv_usec + second.tv_usec;
	if(result.tv_usec >= 1000000) {
		result.tv_sec++;
		result.tv_usec -= 1000000;
	}

	return result;
}

/**
	time_ms
	Given a timeval struct, time_ms returns the time in milliseconds.
	Just a warning, though:  we don't check to see if it fits.
*/
inline int time_ms(struct timeval t)
{
	return (t.tv_usec / 1000) + (t.tv_sec * 1000);
}
