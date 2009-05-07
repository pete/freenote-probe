/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "../config.h"
#include "io/stdout.h"
#include "debug.h"


void io_stdout_init()
{
}

void io_stdout_finish()
{
}

void io_stdout_err(const char *s)
{
	fprintf(stderr, "%s", s);
}
void io_stdout_warning(const char *s)
{
	fprintf(stderr, "%s", s);
}
void io_stdout_info(const char *s)
{
	fprintf(stdout, "%s", s);
}
void io_stdout_debug(const char *s)
{
	fprintf(stderr, "%s", s);
}
