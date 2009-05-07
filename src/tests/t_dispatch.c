/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -g -o tests/t_dispatch tests/t_dispatch.c lib/dispatch.c lib/task.c urlencode.c urldecode.c header-parse.c lib/nv_pair.c crypt/base64/base64.c lib/proto.c lib/fileman.c net-modules/http.c net-modules/dns.c io.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe p_time.c -lssl -lcrypto url-parse.c limits.c
*/
#endif

#include "lib/nv_pair.h"
#include "net-modules/dns.h"
#include "net-modules/http.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include "util.h"
#include "options.h"

#include "lib/proto.h"
#include "lib/fileman.h"
#include "lib/dispatch.h"

#include "p_types.h"
#include "urlencode.h"


int main (int argc, char ** argv)
{

	fm_init_base();
	get_jobs("dev.petta-tech.com");

	return 0;

}

