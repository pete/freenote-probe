/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -o tests/t_http_ssl tests/t_http_ssl.c net-modules/http.c net-modules/dns.c io.c io/stdout.c lib/fileman.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe -lssl -lcrypto -g -Wall -O0 -pipe p_time.c -lssl -lcrypto url-parse.c header-parse.c lib/nv_pair.c lib/cookie.c urldecode.c -lz
*/
#endif

#include "net-modules/http.h"
#include "net-modules/dns.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define DHOST "gmail.google.com"
int main (int argc, char ** argv)
{
	struct timeval tv;
	struct in_addr addr = resolve(DHOST,"4.2.2.1", &tv);
	http_request * req = new_http_req(
			addr,
			0,
			DHOST,
			"/gmail",
			1, NULL, NULL);
	req->version = 1;
	http_get(req);

	fprintf(stdout,"[%s]\n",req->head);
	fprintf(stdout,"[%s]\n",req->body);

	http_req_destroy(req);

	return 0;
}

