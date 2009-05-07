/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -o tests/t_urlstring_to_list -I./ io.c io/syslog.c io/filelog.c options.c urldecode.c tests/t_urlstring_to_list.c lib/nv_pair.c -g -Wall
#endif

#define _GNU_SOURCE
#include <string.h>

#include <stdio.h>
#include <sys/param.h>

#include "lib/nv_pair.h"
#include "urldecode.h"
#include "util.h"

#define str "probe_id=37&dispatch_key=-----BEGIN%20PUBLIC%20KEY-----&poo="

int main ()
{
	nv_list *nvl = nv_urlstring_to_list(str);
	nv_list_dump(nvl);
	nv_list_destroy(nvl);
	return 0;
}

