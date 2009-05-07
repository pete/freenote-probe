/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __HEADER_PARSE_H
#define __HEADER_PARSE_H

/** Look for the value of an http header hdr inside buf (which 
 *  is a string containing an HTTP header) and returns
 *  a pointer to a string which contains its value.
 *  This uses strdup/malloc internally, so remember to free the return
 *  value after you're done using it.
 *  This returns NULL if the header can't be found.
 *  This can handle all 822-style (email, http) headers, even those
 *  with newlines */
char * get_header_dup (const char * hdr, const char * buf);

/** returns 1 if header indicates that the to-be-received file is html
 *  or 0 if it is not */
int is_html(const char *header);

#endif /* __HEADER_PARSE_H */

