/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "../config.h"
#include "io/syslog.h"
#include "debug.h"

#include <syslog.h>

void io_syslog_init()
{
	openlog(PACKAGE_NAME,0,LOG_DAEMON);
}

void io_syslog_finish()
{
	closelog();
}

void io_syslog_err(const char * s)     { syslog(LOG_ERR,"%s",s); }
void io_syslog_warning(const char * s) { syslog(LOG_WARNING,"%s",s); }
void io_syslog_info(const char * s)    { syslog(LOG_INFO,"%s",s); }
void io_syslog_debug(const char * s)   { syslog(LOG_DEBUG,"%s",s); }


