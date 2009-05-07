/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _IO__SYSLOG_H
#define _IO__SYSLOG_H

void io_syslog_init();
void io_syslog_finish();
void io_syslog_err(const char * s);
void io_syslog_warning(const char * s);
void io_syslog_info(const char * s);
void io_syslog_debug(const char * s);

#endif /* _IO__SYSLOG_H */

