/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _COMPAT_H
#define _COMPAT_H 
#include <stdarg.h>
#include <stdio.h>

/* TODO:
 * - see if we can't autotool this away
 * - write/copy a replacement for vsnprintf for systems that don't hve it
 */

#ifdef NEED_VSNPRINTF
#  warning "vsnprintf not available on this system!"
#  warning "vsnprintf not available on this system!"
#  warning "vsnprintf not available on this system!"
#  warning "vsnprintf not available on this system!"
#  warning "vsnprintf not available on this system!"
#  define vsnprintf(str,size,format,ap) vsprintf(str,format,ap)
#endif /* NEED_VSNPRINTF */

#endif /* _COMPAT_H */

