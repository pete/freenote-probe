/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "compat.h"
#include "options.h"
#include "io.h"
#include "io/filelog.h"
#include "io/syslog.h"
#include "io/stdout.h"
#include "util.h"

#define MAX_MSGSIZE 4096

#define _IO_METHOD_OPEN     0x00000001
#define _IO_METHOD_SYSLOG   0
#define _IO_METHOD_FILELOG  1

/* keep track of whether or not our io methods are ready: */
static volatile unsigned io_initialized = 0;

static struct _io_method {
	char * name;
	void (*init)();
	void (*finish)();
	void (*err)(const char *);
	void (*warning)(const char *);
	void (*info)(const char *);
	void (*debug)(const char *);
	unsigned int flags;
} io_method [] = {
	{
		/* .name = */    "syslog",
		/* .init = */    io_syslog_init,
		/* .finish = */  io_syslog_finish,
		/* .err = */     io_syslog_err,
		/* .warning = */ io_syslog_warning,
		/* .info = */    io_syslog_info,
		/* .debug = */   io_syslog_debug,
		/* .flags = */    0
	},
	{
		/* .name = */    "filelog",
		/* .init = */    io_filelog_init,
		/* .finish = */  io_filelog_finish,
		/* .err = */     io_filelog_err,
		/* .warning = */ io_filelog_err,
		/* .info = */    io_filelog_out,
		/* .debug = */   io_filelog_out,
		/* .flags= */    0
	},
	{
		/* .name = */    "stdout",
		/* .init = */    io_stdout_init,
		/* .finish = */  io_stdout_finish,
		/* .err = */     io_stdout_err,
		/* .warning = */ io_stdout_warning,
		/* .info = */    io_stdout_info,
		/* .debug = */   io_stdout_debug,
		/* .flags= */    0
	},
	{ 0 }
};

#define io_printf(target,fmt,...) do { \
	va_list arglist; \
	int i; \
	char * s = NULL; \
	safe_calloc(s,MAX_MSGSIZE); \
	va_start(arglist,fmt); \
	vsnprintf(s,MAX_MSGSIZE,fmt,arglist); \
	va_end(arglist); \
	if (io_initialized) { \
		for (i = 0; io_method[i].name; ++i) \
			if ((io_method[i].flags & _IO_METHOD_OPEN) \
					&& io_method[i].target) \
				io_method[i].target(s); \
	} else \
		printf(s); /* fall back to printf */ \
	safe_free(s); \
} while (0)

void io_err(const char * fmt, ...)     
{ 
	if(read_int_opt("verbosity") > 0)
		io_printf(err,fmt,...);
}

void io_warning(const char * fmt, ...) 
{ 
	if(read_int_opt("verbosity") > 1)
		io_printf(warning,fmt,...);
}

void io_info(const char * fmt, ...)    
{ 
	if(read_int_opt("verbosity") > 2)
		io_printf(info,fmt,...);
}

void io_debug(const char * fmt, ...)   
{ 
	if(read_int_opt("verbosity") > 4) {
		io_printf(debug,fmt,...);   
		fflush(NULL);	/* If we're about to dump core, you want your 
				   message to make it out first, right? */
	}
}

void io_init()
{
	int i;
	const char * log_types = read_str_opt("log-method");

	/* none, output everything to stdout */
	if (strstr(log_types,"none")) {
		io_initialized = 1;
		return;
	}

	for (i = 0; io_method[i].name; ++i)
		if (strstr(log_types,io_method[i].name)) {
			io_method[i].init();
			io_method[i].flags |= _IO_METHOD_OPEN;
			io_initialized = 1;
		}
}

void io_finish()
{
	int i;
	io_initialized = 0;
	for (i = 0; io_method[i].name; ++i) {
		if (io_method[i].flags & _IO_METHOD_OPEN)
			io_method[i].finish();
	}
	fflush(NULL);
}

/**
	io_hexdump
	Dumps dlen bytes of data (pointed to by 'data') to the stream
	specified.  Returns the number of characters printed.
*/
int io_hexdump(FILE *f, const char *data, int dlen)
{
	int r = 0, i;
	for(i = 0; i < dlen; i++) {
		r += fprintf(f, "%02hhx", data[i]);
		if(!(r%78))
			fprintf(f,"\n");
	}
	return r + (dlen / 78);
}

