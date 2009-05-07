/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "probe.h"
#include "lib/fileman.h"
#include "io/filelog.h"
#include "util.h"
#include "err.h"

/* global file descriptors that replace stdout,stderr */
static FILE * out;
static FILE * err;

/** move redirect stderr and stdout to log areas
 */
void io_filelog_init()
{
	mode_t prev = umask(0077);
	char * outlog, * errlog;

	outlog = fm_abs(FM_LOG_OUT);
	out = fopen(outlog,"a");
	if (out == NULL)
		p_exit(PEXIT_FAILURE);

	errlog = fm_abs(FM_LOG_ERR);
	err = fopen(errlog,"a");
	if (err == NULL)
		p_exit(PEXIT_FAILURE);

	fprintf(stdout,"filelog output directed to: %s\n",outlog);
	fprintf(stdout,"filelog errors directed to: %s\n",errlog);
	safe_free(outlog);
	safe_free(errlog);
	
	if(dup2(fileno(out),fileno(stdout))<0)
		p_exit(PEXIT_FAILURE);
	
	if(dup2(fileno(err),fileno(stderr))<0)
		p_exit(PEXIT_FAILURE);
	
	umask(prev);
}

void io_filelog_finish()
{
	fclose(out);
	fclose(err);
}

#define logprint(dst,s) do { \
	time_t t = time(NULL); \
	char date[32] = { 0 }; \
	strftime(date,31,"%b %e %R:%S",localtime(&t)); \
	fprintf(dst,"%s %s",date,s); \
} while (0)

void io_filelog_err(const char * s) { logprint(err,s); }
void io_filelog_out(const char * s) { logprint(out,s); }


