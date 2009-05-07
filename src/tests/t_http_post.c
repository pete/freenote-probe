/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -g -o tests/t_http_post tests/t_http_post.c net-modules/http.c net-modules/dns.c io.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe p_time.c lib/nv_pair.c -lssl -lcrypto url-parse.c url-extract.c p_regex.c

*/
#endif

#include "lib/nv_pair.h"
#include "net-modules/http.h"
#include "net-modules/dns.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main (int argc, char ** argv)
{
	nv_list * nvl = nv_list_init(3);
	struct timeval tv;
	struct in_addr addr = resolve("des.petta-tech.com","4.2.2.1", &tv);
	http_request * req = new_http_req(
			addr,
			8080,
			"des.petta-tech.com",
			"/apps/login",
			0, NULL, NULL);

	nv_list_insert(nvl,"m","login",0);
	nv_list_insert(nvl,"username","eric",0);
	nv_list_insert(nvl,"password","spoonsinjune",0);
	
	http_post (req, nvl);

	fprintf(stdout,"[%s]\n",req->head);
	fprintf(stdout,"[%s]\n",req->body);

	http_req_destroy(req);

	return 0;
}


