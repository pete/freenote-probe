/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _IO_H
#define _IO_H

#include <stdio.h>

/** start logging, redirect stdout, stderr since it's a daemon
 *  also open any sockets that need to be opened and such
 */
void io_init();

/** various printf-like functions of various severity,
 *  they are ordered ascending below, with the most severe
 *  being io_err (error) */
void io_err(const char * fmt, ...);
void io_warning(const char * fmt, ...);
void io_info(const char * fmt, ...);
void io_debug(const char * fmt, ...);

/** close all opened interfaces */
void io_finish();

/** Dumps dlen bytes of data (pointed to by 'data') to the stream
  * specified. */
int io_hexdump(FILE *f, const char *data, int dlen);

#endif /* _IO_H */


