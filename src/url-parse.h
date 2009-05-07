/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	Currently, only conversions involving HTTP work here.  In the future,
	if we add more protocols, there will probably be a handler, that 
	determines the protocol based on the schema part of the URL, etc.
*/

#ifndef _URL_PARSE_H
#define _URL_PARSE_H

#include "net-modules/http.h"
#include "util.h"

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */

typedef struct { 
	char	scheme[1024];
	char	user[1024];
	char	passwd[1024];
	char	host[1024];
	int	port;
	char	path[1024];
} parsed_url;

typedef struct {
	char	*start;
	char	*stop;
} substr;

parsed_url url_parse(char *url);
char *cut_piece(char *dest, substr s);
http_request *url2http(char *url);

#endif
