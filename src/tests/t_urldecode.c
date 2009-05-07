/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#if 0
gcc -o tests/t_urldecode tests/t_urldecode.c urldecode.c -I./ -Wall -pipe io.c io/*.c options.c
*/
#endif /* 0 */

#include "../config.h"
#include "urldecode.h"
#include "util.h"
#include "p_types.h"

#include <stdio.h>
#include <string.h>

int main (int argc, char ** argv)
{
	char * foo = urldecode_dup(argv[1], 0);
	printf("nopreserve: %s\n",foo);
	safe_free(foo);
	
	foo = urldecode_dup(argv[1], 1);
	printf("preserve: %s\n",foo);
	safe_free(foo);

	return 0;
}
