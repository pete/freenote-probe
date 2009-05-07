/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _IO__STDOUT_H
#define _IO__STDOUT_H

void io_stdout_init();
void io_stdout_finish();
void io_stdout_err(const char *s);
void io_stdout_warning(const char *s);
void io_stdout_info(const char *s);
void io_stdout_debug(const char *s);

#endif				/* _IO__STDOUT_H */
