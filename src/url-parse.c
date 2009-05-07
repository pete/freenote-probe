/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	Currently, only conversions involving HTTP work here.  In the future,
	if we add more protocols, there will probably be a handler that 
	determines the protocol based on the schema part of the URL, etc.
*/


#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "net-modules/http.h"
#include "util.h"
#include "url-parse.h"
#include "net-modules/dns.h"
#include "inaddr_none.h"
#include "io.h"
#include "err.h"

static char *url_scheme(char *start, substr *scheme);
static char *url_user_pass(char *start, substr *user, substr *passwd);
static char *url_host(char *start, substr *host);
static char *url_port(char *start, substr *port);
static char *url_path(char *start, substr *path);

static http_request *urlparse(char *url, char *hostname, char *path,
				char *user, char *passwd);

/**
	url2http
	Given a url as a string, it returns a pointer to a dynamically allocated
	http_request struct.

	args:
	url = A string of the following form:  
		<scheme>://<user>:<pass>@<host>:<port>/<url-path>
	where <scheme> is either 'http' or 'https', and all parts are optional
	except the scheme, the host and the path.  So, the minimum is something
	like
		<scheme>://<host>/<url-path>
	Returns a pointer to the http_request struct generated from the string.
*/
http_request *url2http(char *url)
{
	http_request	*nml;
	struct in_addr	addr;
	int		ssl = 0;
	parsed_url	purl;

	if (url == NULL) {
		//print_loc(); io_debug("NULL url passed!\n");
		return NULL;
	}

	if(strncmp(url, "http", 4))  { /* Must start with 'http'. */
		print_loc(); io_debug("Bad URL:  %s\n", url);
		return NULL;
	}
	
	purl = url_parse(url);
	addr = resolve_cached(purl.host, NULL, NULL);
	
	if(!strcmp(purl.scheme, "https"))
		ssl = 1;

	return new_http_req(addr, purl.port, purl.host, purl.path, ssl, 
		purl.passwd, purl.user);
}


/*
	Given a URL, it returns a parsed_url, any fields of which may be null
	if they are not present or parsable in the URL.
*/
parsed_url url_parse(char *url)
{
	substr scheme, user, passwd, host, port, path;
	parsed_url r;
	char *cursor;

	scheme = user = passwd = host = port = path = (substr) { 0, 0 };

	cursor = url_scheme(url, &scheme);
	cursor = url_user_pass(cursor, &user, &passwd);
	cursor = url_host(cursor, &host);
	cursor = url_port(cursor, &port);
	url_path(cursor, &path);

	cut_piece(r.scheme, scheme);
	cut_piece(r.user, user);
	cut_piece(r.passwd, passwd);
	cut_piece(r.host, host);
	cut_piece(r.path, path);

	if(port.start != NULL)
		r.port = atoi(port.start);
	else
		r.port = 0;

	return r;
}

// 'scheme://[user[:passwd]@]host[:port]/path'
static char *url_scheme(char *start, substr *scheme)
{
	scheme->start = start;
	scheme->stop = strchr(start, ':');
	if(scheme->stop) {
		scheme->start = NULL;
		return scheme->stop + 3;
	} else {
		start = NULL;
		return NULL;
	}
}

// '[user[:passwd]@]host[:port]/path'
static char *url_user_pass(char *start, substr *user, substr *passwd)
{
	char *atloc, //It rhymes with Matlock.
	     *colo;

	tryp(start);
	user->start = passwd->start = NULL;
	atloc = strchr(start, '@');
	colo = strchr(start, ':');
	if(atloc == NULL)
		return start;
	
	user->start = start;
	if(colo && colo < atloc) {
		user->stop = colo;
		passwd->start = colo + 1;
		passwd->stop = atloc;
	} else
		user->stop = atloc;
	
	return atloc + 1;
}

// 'host[:port]/path'
static char *url_host(char *start, substr *host)
{
	char *colo, *slash;

	tryp(host->start = start);
	colo = strchr(start, ':');
	slash = strchr(start, '/');

	if(colo && colo < slash) {
		host->stop = colo;
	} else 
		host->stop = slash;

	return host->stop;
}

// '[:port]/path'
static char *url_port(char *start, substr *port)
{
	tryp(start);
	port->start = start + 1;
	if(start[0] != ':') {
		port->start = NULL;
		return start;
	}
	return strchr(start, '/');
}

// '/path'
static char *url_path(char *start, substr *path)
{
	if(start == NULL)
		return NULL;
	path->start = start;
	path->stop = start + strlen(start);
	return path->stop;
}

char *cut_piece(char *dest, substr s)
{
	dest[0] = '\0';
	if(s.start == NULL)
		return NULL;
	if(s.stop == NULL)
		abort();
	
	strncpy(dest, s.start, s.stop - s.start);
	dest[s.stop - s.start] = '\0';

	return dest;
}
