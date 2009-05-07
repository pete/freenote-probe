/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <string.h>
#include <math.h>

#include "err.h"
#include "util.h"
#include "crypt/base64/base64.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <assert.h>

/**
	b64decode
	Decodes a base64-encoded message.  You provide an encoded message,
	and a pointer to somewhere we (the editorial we) can store the length 
	of the decoded message.  A dynamically allocated pointer is returned.
*/
char *b64decode(char *encoded, size_t *newlen)
{
        BIO     *b64, *bmem;
        char    *decoded = NULL;

	if(encoded == NULL) {
		print_loc(); io_debug("NULL data passed!  Bad coder!  Bad!\n");
		return NULL;
	}

	safe_malloc(decoded,*newlen);
        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new_mem_buf(encoded, -1);
	if(bmem == NULL || b64 == NULL) {
		print_loc(); io_debug("Calls to libssl failed!\n");
		abort();  //I don't think this will ever happen.
	}

        bmem = BIO_push(b64, bmem);
        *newlen = BIO_read(bmem, decoded, *newlen);
	BIO_free_all(bmem);

	decoded[*newlen] = '\0';


	return decoded;
}

/**
	b64encode
	Encodes raw data in base64.  Pass the data and the length of the data.
	b64encode returns the base64-encoded data as a dynamically allocated 
	char *.
*/
char *b64encode(char *data, size_t datalen)
{
	BIO	*b64, *bmem;
	BUF_MEM	*bptr;
	char	*encoded = NULL;

	
        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new(BIO_s_mem());
	if(bmem == NULL || b64 == NULL) {
		print_loc(); io_debug("Calls to libssl failed!\n");
		abort();  //I don't think this will ever happen.
	}

        bmem = BIO_push(b64, bmem);
        BIO_write(bmem, data, datalen);
        BIO_flush(bmem);

        BIO_get_mem_ptr(bmem, &bptr);
        BIO_set_close(bmem, BIO_NOCLOSE);

	safe_malloc(encoded, bptr->length);
	memcpy(encoded, bptr->data, bptr->length);

	encoded[bptr->length] = '\0';

	BIO_free_all(bmem);
	BUF_MEM_free(bptr);

	return encoded;
}
