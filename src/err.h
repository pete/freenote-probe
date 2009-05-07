/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	err.h
	A few error handling macros.
*/

#ifndef _ERR_H
#define _ERR_H
#include "io.h"
#include "probe.h"

/*  Debug stuff... */
#define hex_dump_data(d, s) do {					\
	io_debug("DEBUG:  %s:%s():%d:  hexdump follows\n",		\
	__FILE__, __func__, __LINE__);					\
	for(int __i = 0; __i < (s); __i++)				\
		io_debug("%02x", (char)d[__i]);				\
	io_debug("\n");							\
	} while (0)

#define print_loc() io_debug("%s:%s():%d:  ", __FILE__, __func__, __LINE__)

/*  General purpose stuff... */
#define try(x)	do {							\
		if((x) < 0){ 						\
			io_debug("DEBUG:  %s:%s():%d:  try() failed.\n",\
			__FILE__, __func__, __LINE__);			\
			return -1;					\
		} } while(0)

#define tryp(x)	do {							\
		if((x) == NULL) {					\
			io_debug("DEBUG:  %s:%s():%d:  tryp() failed.\n",\
			__FILE__, __func__, __LINE__);			\
			return NULL;					\
		} } while(0)

#define do_or_die(x, y)	do {						\
		if((x) == -1) {						\
		io_debug("DEBUG:  %s:%s():%d:  do_or_die() failed.\n",	\
		__FILE__, __func__, __LINE__);				\
			io_err(y);					\
			p_exit(PEXIT_FAILURE);				\
		} } while (0)

#ifdef DEBUG
#define abort_if_debug() abort()
#else /* DEBUG not defined? */
#define abort_if_debug()
#endif /* DEBUG defined? */
		
#endif /* _ERR_H defined? */

