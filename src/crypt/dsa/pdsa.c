/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <openssl/dsa.h>
#include <openssl/sha.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

#include "crypt/base64/base64.h"
#include "crypt/dsa/pdsa.h"
#include "crypt/ossl_util.h"
#include "lib/fileman.h"
#include "io.h"
#include "err.h"

#include <errno.h>
extern int errno;

static DSA *probe, *dispatch;

static int load_probekeys();
static int load_keys(const char *pubfn, const char *privfn, DSA **dsa);
static int save_keys(const char *pubfn, const char *privfn, DSA *dsa);

/** dsa_init - this is called at startup.  Errors will return -1, and
 *  should be caught by the try() macro
 */
int dsa_init()
{
	char *fn; 

	ou_seed();	/* initialize the random seed, cannot fail */

	dispatch = NULL;

	probe = DSA_generate_parameters(DSA_KEYLEN, NULL, 0, NULL, NULL, NULL, 
		NULL);
	if(probe == NULL) 
		return -1;

	try(load_probekeys());

	dispatch = DSA_generate_parameters(DSA_KEYLEN, NULL, 0, NULL, NULL, 
		NULL, NULL);
	if(dispatch == NULL)
		return 0;	/* no big deal, yet */

	fn = fm_abs(FM_KEY_DISPATCH_PUBLIC);

	if(load_keys(fn, NULL, &dispatch) == -1) {
		DSA_free(dispatch);
		dispatch = NULL;
	}

	free(fn);
	
	return 0;
}

/** dsa_close -  frees up memory used for key signing and verification, run
 * this at exit */
void dsa_close()
{
	DSA_free(probe);
	DSA_free(dispatch);
}

/** 
	dsa_get_ppk
	Returns a dynamically allocated string like the following:
	
	-----BEGIN PUBLIC KEY-----
	...Some, uh, public key...
	-----END PUBLIC KEY-----
	
	or NULL on error.  Note that the pointer is dynamically allocated.
*/
char *dsa_get_ppk()
{
        BIO *fsu = BIO_new(BIO_s_mem());
        BUF_MEM *buf;
        char *realbuf = NULL;

	if(fsu == NULL)
		return NULL;

        PEM_write_bio_DSA_PUBKEY(fsu, probe);
        BIO_flush(fsu);
        BIO_get_mem_ptr(fsu, &buf);

	if((realbuf = malloc(53 + buf->length)) != NULL) {
        	memcpy(realbuf, buf->data, buf->length);
        	realbuf[buf->length] = '\0';
	}

	BIO_free(fsu);
	return realbuf;
}

/**
	load_probekeys
	Loads public and private keys for the probe.  If it cannot load them,
	it generates and saves them.  Returns 0 if we get actual keys and -1 if
	no keys could be loaded or generated.
*/
static int load_probekeys()
{
	int r;
	char *pubfn, *privfn;

	pubfn = fm_abs(FM_KEY_PROBE_PUBLIC);
	privfn = fm_abs(FM_KEY_PROBE_PRIVATE);
	r = load_keys(pubfn, privfn, &probe);

	if(r == -1 && DSA_generate_key(probe)) {
		save_keys(pubfn, privfn, probe);
		r = 0;
	}

	free(pubfn);
	free(privfn);

	return r;
}

/**
	save_dispatch_key
	Saves a base64 encoded key, as well as keeping it for future use.  Pass
	it a key in the form of a null-terminated string.
	Returns -1 for error, 0 for success.
*/
int save_dispatch_key(const char *key)
{
	int r;
	char *pub;

	if(dispatch != NULL)
		return 0;	/* We have ignored you! */

	dispatch = DSA_generate_parameters(DSA_KEYLEN, NULL, 0, NULL, NULL, 
		NULL, NULL);
	if(dispatch == NULL) {
		io_debug("%s:%s():%d  ", __FILE__, __func__, __LINE__);
		io_err("Not enough free memory!  The walls are closing in!\n");
		return -1;
	}

	pub = fm_abs(FM_KEY_DISPATCH_PUBLIC);

	r = !fm_write(FM_KEY_DISPATCH_PUBLIC, key, strlen(key));
	r |= load_keys(pub, NULL, &dispatch);
	free(pub);

	return r;
}

/**
	gen_dsa_sig
	Given some data, the data length, the private key, the private key 
	length, and a buffer to put the 48-byte signature, get_dsa_sig
	generates a dsa signature on the sha1 hash of the data.  Returns 0
	for success, -1 for failure.  
*/
int gen_dsa_sig(const void *data, int dlen, char sigbuf[48])
{
	int	r = -1,
		ssize;
	char	hash[20];
	
	SHA1(data, dlen, hash);
	r = DSA_sign(0x743368, hash, 20, sigbuf, &ssize, probe) - 1;

	return r;
}

/**
	valid_dsa_sig
	Given some data, the data length, the public key, the public key 
	length, and the 48-byte signature, we verify it and return 1 for
	true and 0 for false, or -1 for error.
*/
int valid_dsa_sig(const void *data, int dlen, const char sig[48])
{
	int	r;
	char	hash[20];

	SHA1(data, dlen, hash);
	r = DSA_verify(0xdecafbad, hash, 20, sig, 48, dispatch);
	if(!r) {
		print_loc();
		io_err("Couldn't verify signature!  Info follows:\n");
		io_debug("Hexdump of signature:\n");
		io_hexdump(stdout, sig, 48);
		io_debug("\nHexdump of data follows:\n");
		io_hexdump(stdout, data, dlen);
	}
	return r;
}

/**
	load_keys and save_keys
	Loads or saves the public and/or private keys (depending on whether or
	not the filenames are null) in the pointer to a DSA struct that you
	provide.  Returns 0 for success or -1 for failure.
	
	The macro below is somewhat dirty, but the reason it's there should
	become obvious if you read the code.  It is undef'ed afterwards.
*/
#define LK_act_key(n, c, x, sa, sb)	do { if((n) != NULL) {		\
	f = fopen(n, x);						\
	if(f == NULL) {							\
		io_debug("%s:%s():%d  ", __FILE__, __func__, __LINE__);	\
		io_err(sa, n);						\
		return -1;						\
	}								\
	if((c) == 0) {							\
		fclose(f);						\
		io_debug("%s:%s():%d  ", __FILE__, __func__, __LINE__);	\
		io_err(sb, n);						\
		return -1;						\
	} fclose(f); } } while(0)
/*  See up there, the '(c) == 0' part?  The read functions return NULL and the
    write functions return (int)0 for failure.  So, checking for zero up there
    is fine, but don't go using this for general purpose stuff.
*/
static int load_keys(const char *pubfn, const char *privfn, DSA **dsa)
{
	FILE *f;

	LK_act_key(pubfn, PEM_read_DSA_PUBKEY(f, dsa, NULL, NULL), "r",
		"Could not open %s for reading!\n",
		"Could not read a DSA public key from %s!\n");
	LK_act_key(privfn, PEM_read_DSAPrivateKey(f, dsa, NULL, NULL), "r",
		"Could not open %s for reading!\n",
		"Could not read a DSA private key from %s!\n");
	return 0;
}
static int save_keys(const char *pubfn, const char *privfn, DSA *dsa)
{
	FILE *f;

	LK_act_key(privfn, PEM_write_DSAPrivateKey(f, dsa, NULL, NULL, 0, NULL, NULL), "w", 
		"Could not open %s for writing!\n",
		"Could not write DSA Private key to %s!\n");
	if(privfn != NULL)
		chmod(privfn, 0600);
	LK_act_key(pubfn, PEM_write_DSA_PUBKEY(f, dsa), "w", 
		"Could not open %s for writing!\n",
		"Could not write DSA public key to %s!\n");
	return 0;
}
#undef LK_act_key
