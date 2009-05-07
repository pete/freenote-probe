/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "p_time.h"

int wobbomips()
{
	struct timeval	then = current_time(), 
			now = time_difference(then, current_time());
	int	jinsei = 0,
		shimatta;

	while(now.tv_sec < 2) {
		for(shimatta = 0; shimatta < 1024; shimatta++)
			write(1, NULL, 0);
		now = time_difference(then, current_time());
		jinsei++;
	}
	
	return jinsei / 100;
}
