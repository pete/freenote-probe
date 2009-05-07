/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -o tests/t_nvp -I./ io.c io/syslog.c io/filelog.c options.c tests/t_nvp.c lib/nv_pair.c  -Wall
#endif

#define _GNU_SOURCE
#include <string.h>

#include <stdio.h>
#include <sys/param.h>

#include "lib/nv_pair.h"
#include "util.h"

int main (int argc, char ** argv)
{
	nv_list * nvl;
	char buf[MAXPATHLEN*2];
	unsigned size = 0, i = 0;
	while(fgets(buf,MAXPATHLEN*2,stdin)) {
		++size;
	}
	rewind(stdin);
	
	nvl = nv_list_init(size);
	while(fgets(buf,MAXPATHLEN*2,stdin)) {
		char * tmp = strstr(buf," ");
		printf("o: %s",buf);
		if (!tmp)
			goto end;
		char * name = strndup(buf,strlen(buf) - strlen(tmp));
		char * value = strndup(tmp + 1, strlen(tmp) - 2);
		value[strlen(tmp) - 2] = '\0';
		nv_list_insert(nvl,name,value,i);
		safe_free(name);
		safe_free(value);
		/* printf("name: %s  value: %s\n",nvp[i]->name,nvp[i]->value); */
	end:
		++i;
	}
	nv_list_dump(nvl);

	nv_list_destroy(nvl);
	return 0;
}

