/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "crypt/ossl_util.h"
#include "p_time.h"

#include <openssl/rand.h>

#include <sys/types.h>
#include <unistd.h>

/** initialize the random seed */
void ou_seed()
{
	struct timeval t;
	int seed = getpid() + getppid();
	while(!RAND_status()) {
#ifdef WIN32
		RAND_screen();
#endif
		t = current_time();
		seed += t.tv_usec;
		RAND_add(&seed, sizeof(int), 512);
	}
}
