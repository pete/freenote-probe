/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <sys/types.h>
#include <regex.h>
#include <string.h>

#include "url-extract.h"
#include "url-parse.h"
#include "net-modules/http-recurse.h"
#include "net-modules/http.h"
#include "net-modules/dns.h"
#include "header-parse.h"
#include "p_regex.h"
#include "io.h"

/**
	http_recurse
	Pass us an http request, fool!
*/
int http_recurse(http_request *req, const char *url, int depth)
{
	http_request *sub;
	int 	i,
		total,
		components = 0;
	char 	*c_urls[MAX_TOTAL_URLS];

	io_debug("RECURSE(%d,%d,%d):  %s\n", is_html(req->head), 
		req->status, depth, url);

	if(req->status != 200)
		return 0;
	if(!(is_html(req->head) && depth))
		return 1;	/* 1 component: self */
	if(req->body == NULL) {
		print_loc();  io_debug("Null body passed!\n");
		return 0;
	}

	memset(c_urls, 0, sizeof(char *) * MAX_TOTAL_URLS);
	total = extract_urls(c_urls, MAX_TOTAL_URLS, req->body, url);

	for(i = 0; i < total; i++) {
		sub = url2http(c_urls[i]);
		if(sub == NULL)
			continue;
		if(sub->addr.sin_addr.s_addr == INADDR_NONE) {
			print_loc(); io_debug("Couln't resolve %s.\n", 
				sub->hostname);
		} else if(!http_get(sub))
			components += http_recurse(sub, c_urls[i], depth - 1);
		http_req_destroy(sub);
	}

	for(i = 0; i < MAX_TOTAL_URLS; i++)
		safe_free(c_urls[i]);

	return 1 + components;
}
