/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "probe.h"
#include "url-extract.h"
#include "p_regex.h"
#include "util.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define outer() fprintf(stderr, "%d\n", __LINE__); fflush(NULL)

void readforever(char *x);

void io_err(const char * fmt, ...)
{
	
}

int main(int argc, char *argv[])
{
	int	total, i;
	char	body[1000000];
	char 	*c_urls[MAX_TOTAL_URLS];

	memset(c_urls, 0, sizeof(char *) * MAX_TOTAL_URLS);
	
	readforever(body);
	total = extract_urls(c_urls, MAX_TOTAL_URLS, body, argv[1]);
	for(i = 0; i < total; i++)
		printf("%s\n", c_urls[i]);

	return 0;
}

void readforever(char *x)
{
	int i;
	while(1) {
		i = read(0, x, 1);
		x += i;
		if(i != 1) {
			x[0] = 0;
			break;
		}
	}
}
