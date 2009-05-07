/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */


#include "net-modules/http.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <string.h>

#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <sys/time.h>
#include <time.h>
#include <libgen.h>
#include <zlib.h>

#include "probe.h"
#include "options.h"
#include "util.h"
#include "io.h"
#include "header-parse.h"
#include "net-modules/dns.h"
#include "lib/nv_pair.h"
#include "url-parse.h"
#include "p_types.h"
#include "p_time.h"
#include "inaddr_none.h"

#define _ZBUF                 0x1000
#define HTTP_HEAD_SIZE        0x40000 
#define HTTP_READ_SIZE        0x4000 /* read in 16K chunks */
#define HTTP_BODY_LIMIT       (1024*768)
#define HTTP_REDIRECT_LIMIT   10

#define free_body(req) do { \
	safe_free(req->body); \
	req->body_len = 0; \
} while (0)

#define free_head(req) do { \
	safe_free(req->head); \
	req->head_len = 0; \
} while (0)

#define is_inflatable(s) (!strcasecmp(s,"gzip") || !strcasecmp(s,"deflate"))

#ifdef HTTP_DEBUG
# define http_debug(...) do { print_loc(); io_debug(__VA_ARGS__); } while(0)
#else /* HTTP_DEBUG */
# define http_debug(...)
#endif /* HTTP_DEBUG */
static int http_handle_response (unsigned fd, http_request * req);

/** returns the timeout */
static void get_timeout (struct timeval * to, unsigned to_long)
{
	to->tv_sec = HTTP_SELECT_TIMEOUT;
	to->tv_usec = 0;
	if (to_long)
		to->tv_sec = read_int_opt("network-timeout");
}

/** like strstr, but don't let a zero byte stop it, let limit
 * this will still return the first occurence of needle it finds
 * though */
/* not used
static char * strstr_n(char * haystack, char * needle, size_t limit)
{
	char * ret = NULL;
	size_t nlen = strlen(needle);
	char * start = haystack;
	char *end = haystack + limit - nlen;
	while ((start < end) && ((ret = strstr(start,needle)) == NULL)) {
		start = strchr(start,'\0');
		while(*start == '\0' && start < end)
			start++;
	}
	return ret;
}
*/

/** looks for a CRLF0CRLF sequence in haystack up to limit
 * I'm wasn't too sure about strstr_n above, so I wrote this one instead */
static char * strstr_CRLF0CRLF(char * haystack, size_t limit)
{
	char *c0 = haystack, *end = haystack + limit - 6;
	while (c0 < end) {
		if (c0[0] == '\r' && c0[1] == '\n' && c0[2] == '0' &&
		    c0[3] == '\r' && c0[4] == '\n') /* && c0[5] == '\0') */ {
			http_debug("found CRLF0CRLF!!\n");
			return c0;
		}
		++c0;
	}
	return NULL;
}

/** write everything in a buffer to a file descriptor */
static ssize_t write_from_buffer(unsigned fd, const char * buf, SSL * ssl)
{
	ssize_t seen = 0, w;
	size_t len;

	assert(buf != NULL);
	len = strlen(buf);

	if (ssl) {
		int bfd;
		BIO_get_fd(ssl->rbio,&bfd);
	}
	while (len > 0) {
		size_t num = len < HTTP_READ_SIZE ? len : HTTP_READ_SIZE;
		if (ssl)
			w = SSL_write(ssl, buf + seen, (int)num);
		else
			w = write(fd,buf + seen, num);
		if (w <= 0)
			break;
		len -= w;
		seen += w;
	}
	return seen;
}

/** our select wrapper */
static int r_select (unsigned fd, unsigned to_long)
{
	struct timeval to;
	fd_set rfds;
	int r;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	get_timeout(&to,to_long);

	r = select(fd + 1, &rfds, NULL, NULL, &to);
	if(r == -1) {
		perror("select()");
		switch(errno) {
			case EBADF:
				print_loc();  io_debug("EBADF selecting.\n");
				abort_if_debug();
				break;
			case EINVAL:
				print_loc();  io_debug("EINVAL selecting.\n");
				abort_if_debug();
				break;
			case ENOMEM:
				print_loc();  io_debug("ENOMEM selecting.\n");
				abort_if_debug();
				p_exit(PEXIT_FAILURE);
			case EINTR:
				//Not sure if we should retry in this case.
				print_loc();  io_debug("EINTR selecting.\n"
				"#######################################\n");
				break;
		}
	} else if(r == 0) {
		print_loc(); io_err("select() timeout!\n");
	} //Otherwise, r > 0 and the select worked.
	return r;
}

/* define some common macros for all the read_* functions to use */
/* note: there's no do { .. } while(0) loop around this, to allow
 * break to work */
#define common_read_block() {                                         \
	if (ret == 0) break;                                          \
	if (ret < 0 && errno != EINTR) {                              \
		io_err("problems connecting\n");                      \
		return seen;                                          \
	}                                                             \
	s = ssl ? SSL_read(ssl, buf + seen, (int)num) :               \
	          read(fd, buf + seen, num);                          \
	http_debug("read bytes: %lu:  %lu/%lu\n",s,seen,limit);       \
	if (s == -1 && (errno == EINTR || errno == EAGAIN)) continue; \
	if (s <= 0) break;                                            \
	len -= s;                                                     \
	seen += s;                                                    \
}

/* 	{ int i; for(i=s/8;i!=0;--i) fprintf(stderr,"."); }           \ */

/** a simple read loop */
static ssize_t read_to_buffer(unsigned fd, unsigned char * buf, unsigned len,
		SSL * ssl)
{
	ssize_t seen = 0, last = 0, s = 0;
	size_t limit = len;
	http_debug("len: %u\n",len);
	if (ssl) BIO_get_fd(ssl->rbio,&fd);
	while (len > 0) {
		size_t num = len < HTTP_READ_SIZE ? len : HTTP_READ_SIZE;
		int ret;
		http_debug("start select...");
		ret = r_select(fd, (last == s || s == 0));
		last = (s == 0) ? num : s;
		common_read_block();
	}
/* 	fprintf(stderr,"\n"); */
	return seen;
}

/** this reads chunked encoding, stops if it sees \r\n0\r\n */
static ssize_t read_to_buffer_0(unsigned fd, unsigned char * buf,
		unsigned len, SSL * ssl)
{
	ssize_t seen = 0, last = 0, s = 0;
	size_t limit = len;
	http_debug("len: %u\n",len);
	if (ssl) BIO_get_fd(ssl->rbio,&fd);
	while (len > 0) {
		size_t num = len < HTTP_READ_SIZE ? len : HTTP_READ_SIZE;
		int ret;
		http_debug("start select...");
		ret = r_select(fd, (last == s || s == 0));
		last = (s == 0) ? num : s;
		common_read_block();
		if (NULL!=strstr_CRLF0CRLF(buf,limit))
			break;
	}
	return seen;
}

/* just grab the headers off any connection */
static ssize_t read_header(unsigned fd, unsigned char * buf,
		unsigned len, SSL * ssl)
{
	ssize_t seen = 0, last = 0, s = 0;
	size_t limit = len;
	if (ssl) BIO_get_fd(ssl->rbio,&fd);
	while (len > 0) {
		size_t num = len < HTTP_READ_SIZE ? len : HTTP_READ_SIZE;
		int ret;
		http_debug("start select...");
		ret = r_select(fd, (last == s || s == 0));
		last = (s == 0) ? num : s;
		common_read_block();
		if (NULL!=strstr(buf,"\r\n\r\n"))
			break;
		if (NULL!=strstr(buf,"\n\n"))
			break;
	}
	return seen;
}

#undef common_read_block

/** perform an HTTP GET */
static ssize_t send_http_get_request(unsigned fd, http_request * req)
{
	unsigned char * request = NULL;
	ssize_t ret;
	char * cookies = cookies_strdup(req->cookies, MAX_COOKIES_PER_REQUEST);
	safe_calloc(request,HTTP_HEAD_SIZE);
	
	/* generate a simple http 1.0 request */
	snprintf(request,(HTTP_HEAD_SIZE-1), "GET %s HTTP/1.%d\r\n"
			"Host: %s\r\n"
			"User-Agent: %s\r\n"
			"%s" /* keep-alive */
			"Accept-Encoding: gzip,deflate\r\n"
			"%s"	/* Cookies. */
			"\r\n",
			req->url, req->version, req->hostname,
			read_str_opt("user-agent"),
			(req->version>0 ? "Connection: keep-alive\r\n" : ""),
			(cookies != NULL?  cookies : ""));
	
	http_debug("host: %s header<\n%s>\n",req->hostname,request);
	ret = write_from_buffer(fd,request, req->ssl);
	safe_free(request);
	safe_free(cookies);
	return ret;
}

/** construct a application/x-www-form-urlencoded POST request here: */
static void send_http_post_request(unsigned fd, http_request * req,
		nv_list * nvl)
{
	if (req->head == NULL)
		safe_calloc(req->head,HTTP_HEAD_SIZE);

	req->body = nv_list_to_string(nvl);
	if (req->body == NULL) {
		nv_list_dump(nvl);
		abort();
	}
	
	snprintf(req->head,HTTP_HEAD_SIZE-1,"POST %s HTTP/1.%d\r\n"
			"Host: %s\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: %lu\r\n"
			"\r\n",
			req->url,
			req->version,
			req->hostname, read_str_opt("user-agent"),
			(unsigned long)strlen(req->body));

 	http_debug("host: %s header<\n%s>\n",req->hostname,req->head);
	write_from_buffer(fd,req->head,req->ssl);
	write_from_buffer(fd,req->body,req->ssl);
	free_body(req);
}

/** assumes str is a urlencoded string of name=value pairs */
static void send_http_post_request_str(unsigned fd, http_request * req,
		char *str)
{
	assert(str != NULL);
	
	if (req->head == NULL)
		safe_calloc(req->head,HTTP_HEAD_SIZE);
	
	/* create the HTTP body */
	safe_calloc(req->body, strlen(str) + 1);
	memcpy(req->body, str, strlen(str));
	
	snprintf(req->head,HTTP_HEAD_SIZE-1,"POST %s HTTP/1.%d\r\n"
			"Host: %s\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: %lu\r\n"
			"\r\n",
			req->url,
			req->version,
			req->hostname, read_str_opt("user-agent"), 
			(unsigned long)strlen(req->body));

 	http_debug("host: %s header<\n%s>\n",req->hostname, req->head);
	write_from_buffer(fd,req->head,req->ssl);
	write_from_buffer(fd,req->body,req->ssl);
	free_body(req);
}


/** http_connect - initialize a new HTTP connection */
static int http_connect (http_request * req)
{
        int fd, ret, optval = 0;
	socklen_t optlen = sizeof(optval);
	struct timeval to;
	fd_set wfds, efds;

	if (req->addr.sin_addr.s_addr == INADDR_NONE)
		return -1;
	if ((fd = socket(req->addr.sin_family, SOCK_STREAM, 0)) < 0) {
		io_err("%s:%d: socket creation failed %s\n",
				__FILE__,__LINE__,strerror(errno));
		goto error;
	}

#ifndef BLOCKING_HTTP
	{
		int flags = fcntl(fd, F_GETFL, 0);                                            
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}
#endif /* BLOCKING_HTTP */
	
	ret = connect(fd,(struct sockaddr *)(&(req->addr)),sizeof(req->addr));
	if (ret<0 && errno != EINPROGRESS) {
		io_err("connection to %s failed: %s\n",
				inet_ntoa(req->addr.sin_addr), strerror(errno));
		goto error;
	}

	FD_ZERO(&wfds);
	FD_ZERO(&efds);
	FD_SET(fd, &wfds);
	FD_SET(fd, &efds);
	get_timeout(&to, 1);

	/*	This loop is just to make sure that we retry if we get 
		EINPROGRESS, and handle the other conditions predictably.
		Treating EINPROGRESS as an error was fuckin up our Christmas
		for some https sites.  This is kind of sloppy; I'd like to
		re-structure the function entirely to break it up and make
		it shorter and cleaner.
	*/
	do {
		ret = select(fd + 1, NULL, &wfds, &efds, &to);
		
		if (ret < 0 && errno != EINPROGRESS) { //Bad errors
			print_loc();
			io_debug("problem select()-ing: %s\n", strerror(errno));
			goto error;
		} else if (ret == 0) {	//Timeout
			print_loc();
			io_debug("Select timeout on %d.\n", fd);
			goto error;
		} else if (ret > 0) {	//No errors
			break;
		}
		//Otherwise, it was EINPROGRESS
		print_loc(); io_debug("We did, in fact, get an EINPROGRESS.\n");
		usleep(1000);
	} while(errno == EINPROGRESS);

	getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
	if (optval) {
		print_loc();
		io_err("%s:%d: connection closed\n",__FILE__,__LINE__);
		goto error;
	}

	return fd;
error:
	close(fd);
	return -1;
}

/** this splits and chomps off the part of the header that may have been 
 * intended for the body
 * returns 1 if CRLFCRLF was used to break, 0 if LFLF only */
static void behead_http_req (http_request *req, unsigned len) 
{
	char *split, c;
	size_t offset = 4;
	split = strstr(req->head,"\r\n\r\n");
	if (split == NULL) {
		if ((split = strstr(req->head,"\n\n")) == NULL) {
			print_loc();
			io_warning("Unable to split HEAD from BODY!  "
				   "(Really hard neck?)\n");
			safe_calloc(req->body,req->head_len + 1);
			memcpy(req->body,req->head,req->head_len);
			req->body_len = req->head_len;
			return;
		}
		offset = 2;
	}
	split += offset;
	safe_calloc(req->body,len+1);
	
	c = *split;
	*split = '\0';

	req->body_len = req->head_len - strlen(req->head);
	req->head_len = strlen(req->head);
	*split = c;
	memcpy(req->body, split, req->body_len);
	*split = '\0'; /* truncate the header to where it should be */
	req->head = realloc(req->head,strlen(req->head) + 1);
 	http_debug("header: <%s>\n",req->head);
}

/** reads any chunked http transfer, binary or text */
/* status: working without any known bugs */
/* read the respective RFCs on chunked HTTP transfers to understand this */
static void read_chunked_body_bin (unsigned fd, http_request *req, int keep)
{
	long len = HTTP_READ_SIZE;
	char *c0, *c1;
	ssize_t seen, total = 0;
	char * buf = NULL; /* all reading is done to this temp buffer */

	safe_calloc(buf,HTTP_READ_SIZE+1);
	behead_http_req(req,HTTP_READ_SIZE+1);
	
	if (req->body_len == 0) {
 		http_debug("\n");
		req->body_len = read_to_buffer_0(fd,req->body,
				len,req->ssl);
	}
	seen = total = req->body_len;
	memcpy(buf, req->body, seen);
	c0 = buf;

	while (seen > 0) {
		while (1) {
			if (c0==buf || ((c0>(buf+2))
					&& (seen>2)
					&& (c0[-2]=='\r')
					&& (c0[-1]=='\n')
					&& isxdigit(*c0)
					)) {
				c1 = c0;
				while (isxdigit(*c1) && (c1<(buf+seen)))
					++c1;
				if (((c1+1) < (buf + seen))
						&& (c1[0]=='\r'
						&& c1[1]=='\n')) {
					len = strtoul(c0,NULL,16);
					http_debug("len: %lu\n",len);
					if (len == 0)
						goto end;
					if (seen > len)
						len = seen;
					else {
						/* ("\r\n" * 4) */
						len += 8;
					}
					c0 = (c0 == c1) ? (c0+1) : c1;
				} else
					++c0;
			} else {
				++c0;
				if (c0 >= (buf + seen))
					break;
			}
		}

		if (len > HTTP_READ_SIZE)
			len = HTTP_READ_SIZE;

		/* read stuff into the temporary buffer */
		memset(buf, 0, HTTP_READ_SIZE+1);
 		http_debug("\n");
		seen = read_to_buffer_0(fd, buf, len, req->ssl);
		total += seen;

		/* conditionally copy it to req->body */
		if ((keep!=0) && (seen>0) && 
				((req->body_len+seen)<HTTP_BODY_LIMIT)) {
			req->body = realloc(req->body, req->body_len+seen+1);
			memset(req->body + req->body_len, 0, seen+1);
			memcpy(req->body + req->body_len, buf, seen);
			req->body_len += seen;
		}
		c0 = buf;
	}
end:
	safe_free(buf);
	return;
}

/** turn a chunked HTTP body into a regular one */
/* status: working without any known bugs */
/* read the respective RFCs on chunked HTTP transfers to understand this */
static void unchunk_chunked_body(http_request *req)
{
	char *dest = NULL;
	size_t dlen = 0, len;
	char *c0, *c1, *start;

	c0 = start = req->body;
	while (1) {
		while(((c0-start) < (req->body_len-1)) && !isxdigit(*c0))
			++c0;
		c1 = c0;
		while(((c1-start) < (req->body_len-1)) && isxdigit(*c1))
			++c1;
		if (((c1+1)<(req->body+req->body_len)) &&
					(c1[0]=='\r' && c1[1]=='\n')) {
			len = strtoul(c0,NULL,16);
			if (len == 0) {
				free_body(req);
				req->body_len = dlen;
				req->body = realloc(dest,req->body_len + 1);
				*(req->body+req->body_len) = '\0';
				return;
			}
			c0 = c1+2;
			if ((dlen+len) > req->body_len)
				len = req->body_len - (c0 - start);
			dest = realloc(dest,dlen+len+1); 
			memset(dest+dlen,0,len+1);
			memcpy(dest+dlen,c0,len);
			c0 += len;
			dlen += len;
		} else {
			/* c0 = c1; */ /* I don't think we need this */
			free_body(req);
			req->body_len = c0 - start;
			req->body = realloc(dest,req->body_len + 1);
			*(req->body+req->body_len) = '\0';
/* 			io_err("premature end of chunk: %d\n",req->body_len); */
			return;
		}
	}
}

/** inflates an http req->body into its uncompressed form */
static void http_inflate(http_request *req, const char * tmp)
{
	z_stream z;
	Bytef buf[_ZBUF];
	char *dest = NULL;
	size_t dlen = 0;
	int err;
	int window_bits;
	
	z.zalloc = (alloc_func)Z_NULL;
	z.zfree = (free_func)Z_NULL;
	z.opaque = 0;
	z.next_in = req->body;
	z.avail_in = req->body_len;

	window_bits = (!strcasecmp(tmp,"deflate")) ? -MAX_WBITS : MAX_WBITS+32;

	if ((err = inflateInit2(&z,window_bits)) != Z_OK) {
		io_err("zlib error (%d) on %s%s\n",
				err,req->hostname,req->url);
		free_body(req);
		return;
	}
	
	while (1) {
		int ret;
		z.next_out = &buf[0];
		z.avail_out = _ZBUF;
		ret = inflate(&z, Z_SYNC_FLUSH);
		if (ret == Z_OK || ret == Z_STREAM_END) {
			unsigned seen = _ZBUF - z.avail_out;
			if (((seen+dlen) == 0)
					|| ((seen+dlen) >= HTTP_BODY_LIMIT)) {
				break;
			}
			dest = realloc(dest, dlen + seen + 1);
			memset(dest + dlen, 0, seen + 1);
			memcpy(dest + dlen, &buf[0], seen);
			dlen += seen;
			if (ret == Z_STREAM_END)
				break;
		} else {
			/* -5 == Z_BUF_ERROR  -- don't wait around to handle
			 * this, just error out */
			if (!dest) {
				dlen = 1;
				safe_calloc(dest,dlen);
			}
			break;
		}
	}
	inflateEnd(&z);
	free_body(req);
	req->body_len = dlen;
	req->body = dest;
}

/** reads a chunked http response and automatically inflates it if needed */
static void http_read_chunked_body(unsigned fd,	http_request *req, int keep)
{
	char *tmp = get_header_dup("Content-Encoding",req->head);
	read_chunked_body_bin(fd,req,keep);
	if (keep) {
		unchunk_chunked_body(req);
		if (tmp != NULL) {
			if (is_inflatable(tmp))
				http_inflate(req,tmp);
			safe_free(tmp);
		}
	}
}

/* read everything to a temp buffer and ignore it */
static void http_read_discard (unsigned fd, http_request *req,
		unsigned len)
{
	char * tmp = NULL;
	safe_calloc(tmp,HTTP_READ_SIZE+1);
	while (len > 0) {
		ssize_t s;
		memset(tmp,0,HTTP_READ_SIZE+1);
 		http_debug("\n");
		s = read_to_buffer(fd, tmp,
				(len < HTTP_READ_SIZE ?
				    len : HTTP_READ_SIZE),
				req->ssl);
		if (s<=0)
			break;
		len -= s;
	}
	safe_free(tmp);
}

/** decide how to read a chunked or close request from the server */
static void http_read_length_unknown(unsigned fd, http_request * req,
		int keep)
{
	char * tmp = get_header_dup("Transfer-Encoding",req->head);
	if (tmp != NULL) {
		if (!strncasecmp("chunked",tmp,strlen("chunked")))
			http_read_chunked_body(fd, req, keep);
	} else {
		/* server closed the connection, so just read whatever's
		 * left */
		tmp = get_header_dup("Connection",req->head);
		if ((tmp != NULL) &&
				!strncasecmp("close",tmp,
				strlen("close"))) {
			io_debug("Got \"Connection: close\" header\n");
		} else {
			io_debug("Didn't get \"Connection: close\" header\n");
/* 			io_err("[%s]\n",req->head); */
		}
		behead_http_req(req,req->head_len);
		req->body = realloc(req->body,HTTP_BODY_LIMIT + 1);
		memset(req->body + req->body_len, 0,
				HTTP_BODY_LIMIT - req->body_len + 1);
		http_debug("\n");
		req->body_len += read_to_buffer(fd,
				req->body + req->body_len,
				HTTP_BODY_LIMIT - req->body_len,
				req->ssl);
	}
	safe_free(tmp);
}

/** read a body and keep up to HTTP_BODY_LIMIT */
static void http_read_body_keep (unsigned fd, http_request * req)
{
	char * tmp = NULL;
	tmp = get_header_dup("Content-Length",req->head);

	if (tmp != NULL) {
		unsigned len, total;
		ssize_t seen;
		
		len = total = atoi(tmp);
		safe_free(tmp);
		if (len > HTTP_BODY_LIMIT)
			len = HTTP_BODY_LIMIT;
		behead_http_req(req,len);

		/* read the part we want to keep */
 		http_debug("\n");
		seen = read_to_buffer(fd,
				req->body + req->body_len,
				len - req->body_len, req->ssl);
		req->body_len += seen;

		/* deal with the rest of the junk data */
		if (total > len)
			http_read_discard(fd, req, total-len);
		
		tmp = get_header_dup("Content-Encoding",req->head);
		if (tmp != NULL) {
			if (is_inflatable(tmp))
				http_inflate(req,tmp);
			safe_free(tmp);
		}
	} else 
		http_read_length_unknown(fd, req, 1);
	safe_free(tmp);
}

/** read a body and discard it as it's being read */
static void http_read_body_discard (unsigned fd, http_request * req)
{
	char * tmp = NULL;
	tmp = get_header_dup("Content-Length",req->head);
	if (tmp != NULL) {
		unsigned len = atoi(tmp);
		char * buf = NULL;
		safe_calloc(buf,HTTP_READ_SIZE);
		behead_http_req(req,len);
		while (len > 0) {
			ssize_t s;
			unsigned want = len - req->body_len;
 			http_debug("\n");
			s = read_to_buffer(fd, buf, (want > HTTP_READ_SIZE
					? HTTP_READ_SIZE - req->body_len
					: want), req->ssl);
			if (s<=0)
				break;
			len -= s;
		}
		safe_free(buf);
	} else
		http_read_length_unknown(fd, req, 0);
	free_body(req); /* just in case behead gave it anything */
	safe_free(tmp);
}

/** reads an http body and descides wheter to keep or discard it */
static void http_read_body (unsigned fd, http_request * req)
{
	if (is_html(req->head))
		http_read_body_keep(fd, req);
	else 
		http_read_body_discard(fd, req);
}

/** copy head and body over, handles binary data */
static void headbody_dup(http_request *dest, http_request *src)
{
	if (src->body) {
		free_body(dest);
		dest->body_len = src->body_len;
		safe_calloc(dest->body,dest->body_len + 1);
		memcpy(dest->body,src->body,dest->body_len);
	}
	if (src->head) {
		free_head(dest);
		dest->head_len = src->head_len;
		safe_calloc(dest->head,HTTP_HEAD_SIZE);
		memcpy(dest->head,src->head,dest->head_len);
	}
}

/* handle a HTTP location redirect, waiting on URL parsing 
 * code to parse Location: header
 */
static int http_handle_redirect (unsigned fd, http_request * req)
{
	http_request * newreq = NULL;
	char *url, *persist;
	int ret;
	struct timeval t0;
	
	if(req->redirects >= HTTP_REDIRECT_LIMIT) {
		print_loc();
		io_debug("maximum (%d) number of HTTP redirects exceeded\n",
				HTTP_REDIRECT_LIMIT);
		return -1;
	}

	/* setup a new http request: */
	url = get_header_dup("Location", req->head);
	if(url == NULL) {
		print_loc(); 
		io_debug("Malformed redirect or incomprehensible header!\n"
			 "Header follows:  [%s]\n", req->head);
		return -1;
	}
	print_loc(); io_debug("Redirecting to:  %s\n", url);
	persist = get_header_dup("Connection", req->head);
	newreq = url2http(url);
	if (newreq == NULL) {
		newreq = http_req_dup(req);
		if (newreq == NULL) {
			print_loc(); io_err("Couldn't duplicate request to "
				"redirect to %s!\n", url);
			safe_free(url);
			safe_free(persist);
			return -1;
		}
		ret = http_repath(newreq,url);
		if(ret < 0) {
			http_req_destroy(newreq);
			safe_free(url);
			safe_free(persist);
			return -1;
		}
	}
	safe_free(url);
	newreq->redirects = req->redirects + 1;
	newreq->version = req->version;

	/* run this request */
	t0 = current_time();
	if ((req->version > 0) && (req->ssl_flag == newreq->ssl_flag) &&
			!strcmp(req->hostname,newreq->hostname) &&
			persist && !strcasecmp(persist,"keep-alive")) {
		/* http 1.1 supports persistent connections */
		
		send_http_get_request(fd, newreq);
		ret = http_handle_response(fd, newreq);
	} else {
		newreq->addr.sin_addr = resolve_cached(newreq->hostname, NULL, 
			NULL);
		ret = http_get(newreq);
	}
	
	req->redirect_tv = time_sum(req->redirect_tv,
			time_difference(current_time(),t0));

	/* copy everything back */
	headbody_dup(req,newreq);
	
	req->redirects = newreq->redirects;
	req->status = newreq->status;
	
	http_req_destroy(newreq);
	
	safe_free(persist);
	return ret;
}

static int invalid_header(http_request * req)
{
	if(!strcmp(req->head, ""))
		io_debug("NULL HTTP header!\n");
	else 
		io_debug("No possible HTTP header could be this bad:[%s]\n",
			req->head);
	return -1;
}

/** read the return message from the server and do something with it 
 *  Return value is -1 or the HTTP code.
*/
static int http_handle_response (unsigned fd, http_request * req)
{
	int ret = 0;
	struct timeval t0 = current_time();
	if (req->head == NULL)
		safe_calloc(req->head,HTTP_HEAD_SIZE);
	else
		memset(req->head,0,HTTP_HEAD_SIZE);
	/* read the first byte and time it: */
 	http_debug("\n");
	req->head_len = read_to_buffer(fd, req->head, 1, req->ssl);
	req->first_byte_tv = time_difference(current_time(),t0);
	
	req->head_len += read_header(fd,req->head+req->head_len,
			HTTP_HEAD_SIZE - req->head_len,
			req->ssl);

	if (req->head_len < 16)
		return invalid_header(req);
	
	if(!strncmp(req->head,"HTTP/1.1 ", 9)) {
		ret = atoi(req->head + 9);
	} else if (!strncmp(req->head,"HTTP/1.0 ", 9)) {
		ret = atoi(req->head + 9);
		if (req->version == 1)
			req->version = 0;
	} else if (!strncmp(req->head,"HTTP ", 5))
		/* a broken server just returned "HTTP " */
		ret = atoi(req->head + 5);
	else
		return invalid_header(req);

	parse_cstring(req->cookies, MAX_COOKIES_PER_REQUEST, req->head);
	
	req->status = ret;
		
	print_loc(); io_debug("HTTP code: %d\n",ret);
	switch (ret) {
		case 200:
			http_read_body(fd, req);
			ret = 0;
			break;
		case 301:
		case 302:
			ret = http_handle_redirect(fd, req);
			break;
		case 300:
		case 404:
		default:
			io_debug("\nUnsupported HTTP code: %d [%s]\n",
					ret,req->head);
			break;
	}
	return ret;
}

/** handle SSL initialization here (called when initializing a req object */
static int http_ssl_init (http_request * req)
{
	SSL_METHOD * method = NULL;
	assert(req->ssl_ctx == NULL);
	assert(req->ssl == NULL);
	SSL_library_init();
	SSL_load_error_strings();
	SSLeay_add_all_algorithms();
	SSLeay_add_ssl_algorithms();
	method = SSLv23_client_method();
	if (!method) {
		io_err("%s:%d: error initializing SSL.\n",__FILE__,__LINE__);
		return -1;
	}
	req->ssl_ctx = SSL_CTX_new(method);
	if (!req->ssl_ctx) {
		io_err("%s:%d: error initializing SSL.\n",__FILE__,__LINE__);
		return -1;
	}
	/* don't load CAs or verify for now */
	SSL_CTX_set_verify(req->ssl_ctx,SSL_VERIFY_NONE,NULL);
	req->ssl_flag = 1;
	return 0;
}

static int http_ssl_connect (unsigned fd, http_request * req)
{
	unsigned long e = 0;
	char ebuf[1024] = { 0 };
	if (req->ssl_flag == 2)
		return 0;
	req->ssl = SSL_new(req->ssl_ctx);
	if(!SSL_set_fd(req->ssl,fd)) {
		io_err("SSL: %s\n", ERR_error_string(e,ebuf));
		return -1;
	}
	SSL_set_connect_state(req->ssl);
	if (1 != SSL_connect(req->ssl)) {
		io_err("SSL: %s\n", ERR_error_string(e,ebuf));
		return -1;
	}
	req->ssl_flag = 2;
	return 0;
}

static void http_ssl_destroy(http_request * req)
{
	if (req->ssl_flag) {
		if (req->ssl_ctx)
			SSL_CTX_free(req->ssl_ctx);
		if (req->ssl) {
			if (!SSL_shutdown(req->ssl))
				SSL_shutdown(req->ssl);
			SSL_free(req->ssl);
		}
	}
}

/* -- public functions begin here -- */

void http_req_destroy(http_request * req)
{
	free_head(req);
	free_body(req);
	safe_free(req->hostname);
	safe_free(req->url);
	safe_free(req->password);
	safe_free(req->username);
	http_ssl_destroy(req);
	safe_free(req);
}

/** creates a new HTTP 1.0 request structure */
http_request * new_http_req (
		struct in_addr host, const int port, const char * hostname,
		const char * url, const int ssl, const char * password, 
		const char * username)
{
	struct timeval t0 = { 0, 0 };
	http_request * req = NULL;
	safe_calloc(req,sizeof(http_request));
        
	req->addr.sin_family = AF_INET;
	req->addr.sin_addr = host;
	req->version = 1;
	req->body_len = 0;

	req->body = NULL;
	req->redirects = 0;
	req->status = 0;

	req->redirect_tv = t0;
	req->connect_tv = t0;
	req->first_byte_tv = t0;
	req->page_tv = t0;

	if (!port)
		req->addr.sin_port = htons((ssl ? 443 : 80));
	else
		req->addr.sin_port = htons(port);

	if (username) {
		req->username = strdup(username);
		if (password)
			req->password = strdup(password);
	}

	req->ssl_flag = 0;
	req->ssl_ctx = NULL;
	req->ssl = NULL;
	if (ssl) {
		int ret = http_ssl_init(req);
		if (ret) {
			http_req_destroy(req);
			return NULL;
		}
	}	
	safe_calloc(req->head, HTTP_HEAD_SIZE);

	req->url = strdup(url);
	req->hostname = strdup(hostname);

	init_cookie_list(req->cookies, MAX_COOKIES_PER_REQUEST);
	
	return req;
}

/**
	hreq_transform
	Transforms an http_request based on specified data.  Returns 0 on 
	success, -1 for partial success or error.  
	All parameters are optional.  If a parameter is NULL, then it remains
	unchanged.  
	It's large and ugly.
*/
int hreq_transform(http_request *req, struct in_addr * host, int * port,
	const char * hostname, const char * url, const char * password, 
	const char * username)
{
	char *tmp;
	if(host != NULL)
		req->addr.sin_addr = *host;
	if(port != NULL)
		req->addr.sin_port = *port;
	if(hostname != NULL) {
		tmp = strdup(hostname);
		if(tmp == NULL)
			return -1;
		safe_free(req->hostname);
		req->hostname = tmp;
	}
	if(url != NULL) {
		tmp = strdup(url);
		if(tmp == NULL)
			return -1;
		safe_free(req->url);
		req->url = tmp;
	}
	if(password != NULL) {
		tmp = strdup(password);
		if(tmp == NULL)
			return -1;
		safe_free(req->password);
		req->password = tmp;
	}
	if(username != NULL) {
		tmp = strdup(username);
		if(tmp == NULL)
			return -1;
		safe_free(req->username);
		req->username = tmp;
	}

	return 0;
}

/**
	http_repath
	Given a request and a path, which may be relative, it changes the 
	http_request to the new path.
*/
//SEGV FOO Segfault somewhere in here.  Make sure to build with -ggdb.  My best
//guess is that someone passed it a null new_path or req, but I'm not sure.
//I put in an assertion that should help us out.
int http_repath(http_request *req, const char *new_path)
{
	char path[MAX_URL_LEN];

	assert(new_path != NULL); assert(req != NULL);
	if(new_path[0] == '/')
		return hreq_transform(req, NULL, NULL, NULL, new_path, 
			NULL, NULL);

	strncpy(path, req->url, MAX_URL_LEN);

	if(path[strlen(path) - 1] != '/')
		dirname(path);
	
	while(1) {
		if(strstr(new_path, "./") == new_path)
			new_path += 2;
		else if(strstr(new_path, "../") == new_path) {
			new_path += 3;
			dirname(path);
		} else
			break;
	}

	return hreq_transform(req, NULL, NULL, NULL, 
		strcat(strcat(path, "/"), new_path), NULL, NULL);
}

/** post an HTTP request from a list of name-value pairs */
int http_post (http_request * req, nv_list * nvl)
{
        int fd, ret;
	struct timeval t0 = current_time();
	
	fd = http_connect(req);
	if (req->ssl_ctx) {
		if ((ret = http_ssl_connect(fd,req))!=0)
			return ret;
	}
	
	send_http_post_request(fd, req, nvl);
	req->connect_tv = time_difference(current_time(),t0);
	
	ret = http_handle_response(fd,req);
	
	close(fd);
	req->page_tv = time_difference(current_time(),t0);
	return ret;
}

/** GET method for HTTP 1.0/1.1 */
int http_get (http_request * req)
{
        int fd, ret;
	struct timeval t0 = current_time();

	fd = http_connect(req);
	if(fd == -1)
		return -1;

	if (req->ssl_ctx)
		if ((ret = http_ssl_connect(fd,req))!=0) {
			close(fd);
			return ret;
		}

	send_http_get_request(fd,req);
	req->connect_tv = time_difference(current_time(),t0);

	ret = http_handle_response(fd,req);
	
	close(fd);
	req->page_tv = time_difference(current_time(),t0);
	return ret;
}

/** post an HTTP request from a string */
/** Return value is either the HTTP code, or -1 for error. */
int http_post_str (http_request * req, char *str)
{
        int fd, ret;
	struct timeval t0 = current_time();
	
	fd = http_connect(req);
	if(fd == -1)
		return -1;

	if (req->ssl_ctx) {
		if ((ret = http_ssl_connect(fd,req))!=0)
			return ret;
	}
	
	send_http_post_request_str(fd, req, str);
	req->connect_tv = time_difference(current_time(),t0);
	
	ret = http_handle_response(fd, req);
	
	close(fd);
	req->page_tv = time_difference(current_time(),t0);	
	return ret;
}

/** creates a new HTTP 1.0 request object based on an old one */
http_request * http_req_dup(http_request *req)
{
	http_request * newreq = NULL;
	int tmp;

	safe_calloc(newreq,sizeof(http_request));
       	 
	newreq->addr = req->addr;
	newreq->version = req->version;
	newreq->redirects = req->redirects;
	newreq->status = req->status;
	
	newreq->redirect_tv = req->redirect_tv;
	newreq->connect_tv = req->connect_tv;
	newreq->first_byte_tv = req->first_byte_tv;
	newreq->page_tv = req->page_tv;
	
	newreq->body = NULL;
	newreq->head = NULL;
	newreq->url = NULL;
	newreq->hostname = NULL;
	newreq->username = NULL;
	newreq->password = NULL;
	
	if (req->username) {
		newreq->username = strdup(req->username);
		if (req->password)
			newreq->password = strdup(req->password);
	}

	newreq->ssl_flag = req->ssl_flag;
	newreq->ssl_ctx = NULL;
	newreq->ssl = NULL;
	if (req->ssl_ctx) {
		tmp = http_ssl_init(newreq);
		if (tmp) {
			print_loc(); io_err("Couldn't initialize SSL!\n");
			http_req_destroy(newreq);
			return NULL;
		}
	}

	headbody_dup(newreq,req);

	if (req->url)
		newreq->url = strdup(req->url);
	if (req->hostname)
		newreq->hostname = strdup(req->hostname);
	
	return newreq;
}

