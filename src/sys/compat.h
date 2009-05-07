/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _SYS__COMPAT_H
#define _SYS__COMPAT_H
#include "sys/unix.h"

#ifndef daemonize
#  define daemonize() unix_daemonize()
#endif /* daemonize */


#ifndef strnlen
# define strnlen(str,max) __fn_strnlen(str,max)
#endif /* !defined strnlen */



#endif /* _SYS__COMPAT_H */

