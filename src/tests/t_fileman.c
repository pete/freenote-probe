/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -o tests/t_fileman tests/t_fileman.c lib/fileman.c net-modules/http.c net-modules/dns.c io.c io/syslog.c io/filelog.c options.c -Wall -lnsl -I./ lib/libares/*.c -Ilib/libares -pipe p_time.c lib/nv_pair.c -lssl -lcrypto

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

#include "util.h"
#include "lib/fileman.h"

int main (int argc, char ** argv)
{

	char		*buf;
	long		fsize;
	unsigned int	probe_id;

	fm_init_base();	

	buf = fm_read(FM_ID_PROBE, fm_size(FM_ID_PROBE));
	sscanf(buf, "%u", &probe_id);
	free(buf);

	printf("probe_id: %i\n", probe_id);

	printf("public key: \n");

	fsize = fm_size(FM_KEY_PROBE_PUBLIC);
	buf = fm_read(FM_KEY_PROBE_PUBLIC);

	fwrite(buf, fsize, 1, stdout);

	if (!fm_write("test.pub", buf, fsize)) {
		printf("couldn't write test.pub\n");
	}

	free(buf);


	return 0;

}

