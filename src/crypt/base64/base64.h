/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __BASE64_H
#define __BASE64_H

char *b64decode(char *encoded, size_t *newlen);
char *b64encode(char *data, size_t datalen);

#endif
