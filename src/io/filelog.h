/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _IO__FILELOG_H
#define _IO__FILELOG_H

void io_filelog_init();
void io_filelog_finish();

void io_filelog_err(const char * s);
void io_filelog_out(const char * s);

#endif /* _IO__FILELOG_H */


