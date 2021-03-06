/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc-3.4 -g -o tests/t_http_limit tests/t_http_limit.c net-modules/http.c net-modules/dns.c io.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe p_time.c -lssl -lcrypto url-parse.c header-parse.c lib/nv_pair.c limits.c urldecode.c
*/
#endif

#include "net-modules/http.h"
#include "net-modules/dns.h"
#include "header-parse.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define DHOST "des.petta-tech.com"
int main (int argc, char ** argv)
{
	struct timeval tv;
	struct in_addr addr = resolve(DHOST,"4.2.2.1", &tv);
	int fd;
	char * tmp = NULL;
	http_request * req = new_http_req(
			addr,
			80,
			DHOST,
			"/eric/debian/custom/tla_20041106.normal.orig.tar.gz",
			0, NULL, NULL);
	req->version = 1;
	fprintf(stdout,"return: %d\n", http_get(req));
	fprintf(stdout,"[%s]\n",req->head);
	tmp = get_header_dup("Content-Length",req->head);
	fd = creat("foo.tar.gz",0666);
	{
		int len = atoi(tmp);
		while(len > 0) {
			int wr = write(fd,req->body,len);
			if (wr<=0)
				break;
			len -= wr;
		}
		close(fd);
	}
	free(tmp);
	http_req_destroy(req);
	return 0;
}

