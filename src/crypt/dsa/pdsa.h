/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __PDSA_H
#define __PDSA_H

#include <openssl/bn.h>
#include <openssl/dsa.h>

#define DSA_KEYLEN 784
#define PUBKEY_OPT 0
#define PRIVKEY_OPT 1

/** dsa_init - this is called at startup.  Errors will return -1, and
 *  should be caught by the try() macro
 */
int dsa_init();

/** dsa_close -  frees up memory used for key signing and verification, run
 * this at exit */
void dsa_close();

/** gen_dsa_sig
 * Given some data, the data length, the private key, the private key 
 * length, and a buffer to put the 48-byte signature, get_dsa_sig
 * generates a dsa signature on the sha1 hash of the data.  Returns 0
 * for success, -1 for failure.  
 */
int gen_dsa_sig(const void *data, int dlen, char sigbuf[48]);

/** valid_dsa_sig
 * Given some data, the data length, the public key, the public key 
 * length, and the 48-byte signature, we verify it and return 1 for
 * true and 0 for false, or -1 for error.
 */
int valid_dsa_sig(const void *data, int dlen, const char sig[48]);

/** dsa_get_ppk
 * Returns a dynamically allocated string like the following:
 * 
 * -----BEGIN PUBLIC KEY-----
 * ...Some, uh, public key...
 * -----END PUBLIC KEY-----
 * 
 * or NULL on error.  Note that the pointer is dynamically allocated.
 */
char *dsa_get_ppk();

/** save_dispatch_key
 * Saves a base64 encoded key, as well as keeping it for future use.  Pass
 * it a key in the form of a null-terminated string.
 * Returns -1 for error, 0 for success.
 */
int save_dispatch_key(const char *key);

#endif /* __PDSA_H defined? */
