/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _HTTP_RECURSE_H
#define _HTTP_RECURSE_H

#include "net-modules/http.h"

int http_recurse(http_request *req, const char *url, int depth);

#endif /* _HTTP_RECURSE_H defined? */
