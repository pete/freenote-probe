/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
gcc -g -Wall tests/t_b64.c crypt/base64/base64.c -I. -lssl -lcrypt && ./a.out
*/
#include <stdio.h>

#include "crypt/base64/base64.h"

int main(void)
{
	int len;
	char data[] = "For a simple illumination model this means that we"
		" only apply the ambient term\nfor that light source."
		" First Intersection point in the shadow of the second\nobject"
		"  Also, when a ray hits an object, a reflected ray is gene"
		"rated which i\ntested against all of the objects in the s"
		"cene.\n";
	char *enc, *dec;

	enc = b64encode(data, strlen(data));
	dec = b64decode(enc, &len);

	printf("Original data(%d):\n%s\n\nEncoded data(%x):\n%s\n\nDecoded data"
		"(%x,%d):\n%s\n\n", strlen(data), data, enc, enc, dec, len, 
		dec);
	return 0;
}
