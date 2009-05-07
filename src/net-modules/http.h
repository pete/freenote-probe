/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _NET_MODULE__HTTP_H
#define _NET_MODULE__HTTP_H

#include <limits.h>
#include <time.h>

#ifdef WIN32
# include <winsock.h>
#else
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/time.h>
#endif /* WIN32 */

/*  Sometimes it's defined already, sometimes it's not.  */
#ifndef OPENSSL_NO_KRB5
#	define OPENSSL_NO_KRB5 1
#endif 

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#include "lib/nv_pair.h"
#include "lib/cookie.h"

#define MAX_URL_LEN      2048
#define MAX_COOKIES_PER_REQUEST 10  /* 100% arbitrary. Cookies are ~3k each */
#define HTTP_SELECT_TIMEOUT 5

/** This is the structure which describes an HTTP request */
typedef struct _http_request {
        struct sockaddr_in addr;
	unsigned redirects;      /* number of redirects we're nested in */
	unsigned version;        /* 1 for 1.1 (allows persistent connections)
				    0 for 1.0 */
	unsigned ssl_flag;
	
	struct timeval redirect_tv; 
	struct timeval connect_tv;     /* connection setup time */
	struct timeval first_byte_tv;  /* time to first byte */
	struct timeval page_tv;        /* time to receive a page */
	
	int status;              /* RFC1945 status code, i.e., 200, 404, 302 */
	size_t body_len;
	size_t head_len;
	char * hostname;
	char * password;
	char * username;
	char * url;              /* partial (no host) URL to retrieve (host) */
	char * head;             /* HTTP header goes here */
	char * body;             /* where the HTTP body goes for GET */
	SSL_CTX * ssl_ctx;
	SSL * ssl;
	
	cookie cookies[MAX_COOKIES_PER_REQUEST];
} http_request;

/** creates a new HTTP 1.0 request structure
 *  This returns a new struct pointer which you need to free once you're
 *  done using it */
http_request * new_http_req(
		struct in_addr host, const int port, const char * hostname,
		const char * url, const int ssl, const char * password, 
		const char * username);

/** duplicate an existing http request structure,
 *  This returns a new struct pointer which you need to free once you're
 *  done using it */
http_request * http_req_dup(http_request *req);

/** Transforms an http_request based on specified data.  Returns 0 on 
 * success, -1 for partial success or error.  
 * All parameters are optional.  If a parameter is NULL, then it remains
 * unchanged. */
int hreq_transform(http_request *req, struct in_addr * host, int * port,
        const char * hostname, const char * url, const char * password,
	const char * username);

/** Given a request and a path, which may be relative, it changes the 
 * http_request to the new path. */
int http_repath(http_request *req, const char *new_path);

/** destroys and frees memory used by an http request */
void http_req_destroy(http_request * req);

/** GET method for HTTP (retrieves req->url from req->host) */
int http_get(http_request * req);

/** post an HTTP request from a nv_list object (urlencoded name=value pairs) */
int http_post(http_request * req, nv_list * nvl);

/** post an HTTP request from a string (urlencoded name=value pairs) */
int http_post_str(http_request * req, char *str);

#endif /* _NET_MODULE__HTTP_H */

