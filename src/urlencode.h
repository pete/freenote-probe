/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __URLENCODE_H
#define __URLENCODE_H

/* see url(7) for details */

/** returns URL-encoded string pointer, which needs to be freed
 *  after usage.  If set, the preserve_reserved option will prevent
 *  characters like:
 *    ';', '/', '?', ':', '@', '&', '=', '+', '$', ',', '\0' 
 *  from being url-encoded. */
char * urlencode_dup(const char *str, unsigned preserve_reserved);

#endif /* __URLENCODE_H */

