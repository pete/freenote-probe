/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/* tested on linux & solaris */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <utmp.h>

#include "io.h"

time_t get_uptime (void);

int main (int argc, char *argv[])
{

	time_t uptime;

	uptime = get_uptime();

	io_debug("uptime = %i\n", uptime);

}

time_t get_uptime (void)
{
	struct utmp *ut;
	time_t boot_time;

	setutent();

	while (ut = getutent())
	{
	
		if (ut->ut_type == BOOT_TIME)
		{
			boot_time = ut->ut_time;		
		}	
	
	}

	endutent();

	if (boot_time)
		return (time(NULL) - boot_time);
	else
		return 0;


}
