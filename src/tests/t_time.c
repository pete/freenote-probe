/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	Tests p_time.h and p_time.c

*/

/*
gcc -DDEBUG -Wall -I. -Ilib tests/t_time.c p_time.c
*/

#include "p_time.h"
#include <stdio.h>
#include <time.h>

int main(void)
{
	struct timeval t;
	struct timeval beginning = current_time();
	float m;	/* Used for a fun calculation. */

	t = current_time();
	printf("Current time in seconds since the epoch:  %d.%06ds\n",
		t.tv_sec, t.tv_usec);

	t = time_difference(current_time(), t);
	printf("Since that last printf, %d.%06d seconds have passed.\n",
		t.tv_sec, t.tv_usec);

	printf("Now we'll sleep for a second and time it.  (whee...)\n");
	t = current_time();
	sleep(1);
	t = time_difference(current_time(), t);
	m = 100.0 * ((float)t.tv_sec - 1.0 + (float)(t.tv_usec) / 1000000.0);
	printf("'sleep(1)' actually took %d.%06ds; it was off by %f%%.\n",
		t.tv_sec, t.tv_usec, m);
	printf("thank you for your time...your %dms to be more precise.\n",
		time_ms(time_difference(current_time(), beginning)));

	return 0;
}
