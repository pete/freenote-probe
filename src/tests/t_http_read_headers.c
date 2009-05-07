/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -g -o tests/t_http_read_headers tests/t_http_read_headers.c -Wall -lnsl -I./ -pipe header-parse.c io.c io/*.c options.c lib/fileman.c -DDEBUG=1
*/
#endif

#include "util.h"
#include "header-parse.h"
#include <sys/param.h>
#include <string.h>
#include <stdio.h>

int main (int argc, char ** argv)
{
	char *buf = NULL, *foo = NULL;
	safe_malloc(buf,MAXPATHLEN*2);
	safe_malloc(foo,MAXPATHLEN*4);
	off_t off = 0;
	char *moo;

	if (buf == NULL || foo ==NULL) {
		io_err("WTF\n");
		return 0;
	}
	while(fgets(buf,MAXPATHLEN*2,stdin)) {
		strcat(foo + off,buf);
		off += strlen(buf);
	}
	fprintf(stderr,"buf[%s]\n",foo);
	moo = get_header_dup("Keywords", foo);
	printf("moo: %s\n",moo);
	safe_free(moo);
	safe_free(buf);
	safe_free(foo);
	return 0;
}


