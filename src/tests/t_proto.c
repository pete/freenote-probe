/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -g -o tests/t_proto tests/t_proto.c urlencode.c urldecode.c header-parse.c lib/nv_pair.c crypt/base64/base64.c lib/proto.c lib/fileman.c net-modules/http.c net-modules/dns.c io.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe p_time.c -lssl -lcrypto url-parse.c limits.c
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

#include "p_types.h"
#include "urlencode.h"

int main (int argc, char ** argv)
{
	struct proto_mesg_t	pm, pmr;
        char                    *ppk = NULL, *ppke;
	nv_list			*nvl;
	
	fm_init_base();

	/* must have a valid DSA public key in opt(base-dir)
	   named probe.pub */

        ppk = fm_read(FM_KEY_PROBE_PUBLIC);

	ppke = urlencode_dup(ppk, 0);

	nvl = nv_list_init(4);

	nv_list_insert(nvl, "team_name", "peachpit", 0);
	nv_list_insert(nvl, "pin_code", "5555", 0);
	nv_list_insert(nvl, "os_type", "Linux", 0);
	nv_list_insert(nvl, "pub_key", ppke, 0);

	proto_mesg_build(&pm, FN_TYPE_CHECKIN, 0, nvl);
	proto_mesg_send(&pm, &pmr, FN_DISPATCH);

	nv_list_destroy(nvl);

	printf("CHECKIN:\n");
	proto_dump(&pm);	
	proto_dump(&pmr);	
	
	proto_get_nvl(&pmr, nvl);
	nv_list_dump(nvl);
	nv_list_destroy(nvl);

	proto_mesg_destroy(&pm);
	proto_mesg_destroy(&pmr);

	safe_free(ppk);

	return(0);

}

