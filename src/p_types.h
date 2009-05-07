/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _P_TYPES_H
#define _P_TYPES_H

#include <sys/types.h>
#include "../config.h"

#ifndef uint8
typedef unsigned char	uint8;
#endif /* uint8 */

#ifndef sint8
typedef signed char	sint8;
#endif /* sint8 */

#if SIZEOF_SHORT == 2
#  ifndef uint16
typedef unsigned short 	uint16;
#  endif /* uint16 */
#  ifndef sint16
typedef signed short	sint16;
#  endif /* sint16 */
#elif SIZEOF_INT == 2
#  ifndef uint16
typedef unsigned int 	uint16;
#  endif /* uint16 */
#  ifndef sint16
typedef signed int	sint16;
#  endif /* sint16 */
#endif

#if SIZEOF_INT == 4
#  ifndef uint32
typedef unsigned int 	uint32;
#  endif /* uint32 */
#  ifndef sint32
typedef signed int	sint32;
#  endif /* sint32 */
#elif SIZEOF_LONG == 4
#  ifndef uint32
typedef unsigned long 	uint32;
#  endif /* uint32 */
#  ifndef sint32
typedef signed long	sint32;
#  endif /* sint32 */
#endif

#ifndef word
#  ifdef __UWORD_TYPE 
typedef __UWORD_TYPE word;
#  else /* !defined(__UWORD_TYPE) */
typedef unsigned long word;
#  endif /* __UWORD_TYPE */
#endif /* defined word */

#endif /* _P_TYPES_H */

