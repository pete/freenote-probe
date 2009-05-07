/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "header-parse.h"
#include "util.h"

#define BROKEN_HEADER -1
#define NORMAL_HEADER 0
#define LFLF_HEADER 1

static int header_type(const char *header, char **split);

/* 	get_header_dup
	Given a field to search for (hdr) and an http header (buf), this
	function returns the value of that field from the header.  It very 
	badly needs to be re-written.
	See the header-parse.h file for more details.
*/
char * get_header_dup (const char * hdr, const char * buf)
{
	const char *c = buf;
	char *ret = NULL;
	char *split;
	char *h = NULL;
	int len, cr;

	assert(hdr != NULL);
	assert(buf != NULL);

	/* Sorry to contribute to the mess instead of fixing it. 
	   There's a pretty bad bug I have to fix and I don't have time to
	   re-write it.
	*/
	switch(header_type(buf, &split)) {
		case NORMAL_HEADER:
			cr = 1; break;
		case LFLF_HEADER:
			cr = 0; break;
		case BROKEN_HEADER:
			print_loc(); io_debug("Broken header!\n");
			break;
	}

	if (cr) {
		c = strstr(buf,"\r\n");
		if (c == NULL)
			return NULL;
		safe_calloc(h, strlen(hdr) + 4);
		strcat(h,"\r\n");
	} else {
		c = strstr(buf,"\n");
		if (c == NULL)
			return NULL;
		safe_calloc(h, strlen(hdr) + 3);
		strcat(h,"\n");
	}

	strcat(h,hdr);
	strcat(h,":");
	len = strlen(h);

	if (cr) {
		/* normal CRLF line breaks */
		while(c <= split) {
			if ((ret == NULL) && (0 == strncasecmp(c, h, len))) {
				char * new;
				c += len;
				new = strstr(c,"\r\n");
				if (!new)
					goto end;
				while (isspace(*c)) ++c;

				safe_calloc(ret,(size_t)(new - c + 1));
				strncpy(ret,c,(size_t)(new - c));
				
				c = strstr(new,"\r\n");
				if (!c)
					goto end;
				c+=2;
			} else if (ret!=NULL 
				 && isspace(*c) &&
					c[-2]=='\r' && c[-1]=='\n' &&
					((c-buf) > 3) &&
					c[-4]!='\r' && c[-3]!='\n') {
				char *new = strstr(c,"\r\n");
				size_t len, nlen;
				if (!new)
					goto end;
				while (isspace(*c)) ++c;
				len = strlen(ret);
				nlen = new - c;
		
				ret = realloc(ret, (len + nlen + 3));
				memset((ret + len), 0, (nlen + 3));
				strcat(ret," ");
				strncat(ret,c,nlen);
				c = strstr(new,"\r\n");
				if (!c)
					goto end;
				c+=2;
			} else {
				c = strstr(c+2,"\r\n");
				if (!c)
					goto end;
			}
		}	
	} else {
		/* the strange server uses LF-only */
		while(c <= split) {
			if ((ret == NULL) && (0 == strncasecmp(c, h, len))) {
				char * new;
				c += len;
				new = strchr(c,'\n');
				if (!new)
					goto end;
				while (isspace(*c)) ++c;

				safe_calloc(ret,(size_t)(new - c + 1));
				strncpy(ret,c,(size_t)(new - c));
				
				c = strchr(new,'\n');
				if (!c)
					goto end;
				c++;
			} else if (ret!=NULL 
				 && isspace(*c) &&
					c[-1]=='\n' &&
					((c-buf) > 1) &&
					c[-2]!='\n') { /**/
				char *new = strchr(c,'\n');
				size_t len, nlen;
				if (!new)
					goto end;
				while (isspace(*c)) ++c;
				len = strlen(ret);
				nlen = new - c;
		
				ret = realloc(ret, (len + nlen + 2));
				memset((ret + len), 0, (nlen + 2));
				strcat(ret, " ");
				strncat(ret, c, nlen);
				c = strchr(new, '\n');
				if (!c)
					goto end;
				c++;
			} else {
				c = strchr(c+1,'\n');
				if (!c)
					goto end;
			}
		}	
	}
end:
	safe_free(h);
	return ret;
}	

/*
 is_html
 Provided an html header, is_html() checks to see if the body that
 accompanies the header would be able to be parsed by the ghetto html
 parser, or if we're just wasting our time(2).
*/
int is_html(const char *header)
{
	int 	r = 0;
	char 	*type;

	if(header == NULL) {
		io_err("FIX THIS BUG:  NULL HEADER!!\n"
			"LOOK UP ABOVE TO FIND THE URL THAT IS GIVING US "
			"THIS PROBLEM!!\n");
		return 0;
	}
	
	type = get_header_dup("Content-type", header);
	if(type == NULL)
		return 0;

	if(strstr(type, "html"))
		r = 1;

	safe_free(type);
	return r;
}

static int header_type(const char *header, char **split)
{
	if(*split = strstr(header,"\r\n\r\n")) {
		return NORMAL_HEADER;
	} else if (*split = strstr(header,"\n\n")) {
		return LFLF_HEADER;
	} else if (strrchr(header, '\n') == (strrchr(header, '\r') + 1)) {
		/* This means that there's apparently no body, but the header
		   is normal otherwise.  Doubleclick has done this to us before
		   during redirects.
		*/
		*split = strrchr(header, '\r');
		return NORMAL_HEADER;
	} else if (*split = strrchr(header, '\n')) {
		/* I've never actually _seen_ this, but after the Doubleclick
		   thing, it seems like a good idea to try to account for it.
		*/
		print_loc(); io_debug("Possibly bad header.\n");
		return LFLF_HEADER;
	} else {
		print_loc(); io_debug("This header can't be right.\n");
		return BROKEN_HEADER;
	}
}
