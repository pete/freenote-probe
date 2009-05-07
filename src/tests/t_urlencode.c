/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -g -o tests/t_urlencode tests/t_urlencode.c urlencode.c -I./ -Wall -pipe io.c io/*.c options.c
*/
#endif /* 0 */

#include "../config.h"
#include "urlencode.h"
#include "util.h"
#include "p_types.h"

#include <stdio.h>
#include <string.h>

int main (int argc, char ** argv)
{
	char * foo = urlencode_dup(argv[1], 0);
	printf("nopreserve: %s\n",foo);
	safe_free(foo);
	
	foo = urlencode_dup(argv[1], 1);
	printf("preserve: %s\n",foo);
	safe_free(foo);

	return 0;
}

