/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __URLDECODE_H
#define __URLDECODE_H

/** decode a URL encoded str, this returns a pointer to a newly allocated
 *  string, so be sure to free it */
char * urldecode_dup(const char *str);

#endif /* __URLDECODE_H */
