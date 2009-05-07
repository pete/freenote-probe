/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	work.h
*/
#ifndef _WORK_H
#define _WORK_H

#include "lib/task.h"
#include "net-modules/http.h"
#include "io.h"

int process_tasks();
void populate_handlers();

#endif /* _WORK_H defined? */
