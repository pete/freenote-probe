/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	See main(), waaaaaay down there.
gcc -Wall tests/t_cookie.c lib/cookie.c -I. && echo -n type the url:  && read URL && lynx -source -mime_header $URL | ./a.out
*/
#include <unistd.h>
#include <stdio.h>
#include "lib/cookie.h"

int main(void)
{
	cookie clist[100];
	int total;
	char *buf;
	char msndotcom[16 * 1024];

	read(0, msndotcom, 16 * 1024);
	total = parse_cstring(clist, 100, msndotcom);
	buf = cookies_strdup(clist, 100);
	printf("The cookies as a string:  %s", buf);
	printf("There were %d cookies found:\n", total);
	fflush(stdout);
	write_cookies(1, clist, 100);

	return 0;
}
