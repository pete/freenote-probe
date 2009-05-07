/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include "probe.h"
#include "url-extract.h"
#include "p_regex.h"
#include "util.h"
#include "io.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

static void append_match(regmatch_t match, char *s, int max, void **urls);
static char *get_domain(const char url[]);
static char *get_path(const char url[]);
static char *complete_url(char *url, char domain[], char path[]);
static void urls_only(char line[], regmatch_t list[], int max);

/**
	extract_urls
	Extracts the urls from a set of lines, and puts them into an array.
	args:
		urls = an array of strings.  This is
			where the data is returned, so it must be writable.
		maxurls = the maximum number of urls that can be stored
		file = a NULL-terminated HTML file.  This is where the 
			urls are pulled from.
		base_url = the base url, used to generate complete urls for 
			lines like '<img src="../top.jpg">'
		
*/
int extract_urls(char **urls, int maxurls, char *file, const char base_url[])
{
	regmatch_t regs[MAX_URLS_PER_LINE];
	int i, total = 0;
	char *domain = get_domain(base_url), *path = get_path(base_url);

	if (url_cut(file, regs, MAX_URLS_PER_LINE))
		total = regex_iterate(regs, file, maxurls, (void **) urls,
				      append_match);

	if (MAX_TOTAL_URLS < maxurls)
		maxurls = MAX_TOTAL_URLS;

	for (i = 0; i < maxurls && urls[i]; i++) {
		urls[i] = complete_url(urls[i], domain, path);
	}

	safe_free(domain);
	safe_free(path);

	return total;
}

/*
	complete_url
	Takes a url, a domain, and a path.  Returns a dynamically allocated
	args:
		url = the url to complete.  is free()d by complete_url
		domain = a domain to which an absolute path can be appended
				to create a valid url.
		path = the path of the cwd.  
	returns a pointer to some malloc()ed memory containing the new url.
*/
static char *complete_url(char *url, char domain[], char path[])
{
	int i, len;
	char *new = url;
	
	assert(url != NULL);
	assert(domain != NULL);
	assert(path != NULL);
	
	len = strlen(url);

	for (i = 0; i < len; i++)
		if (url[i] == ':')	/* rfc 1738, section 2.2, "Reserved" */
			return url;

	len += strlen(domain);	/* We need at least the domain+url. */

	if (url[0] == '/') {	/* It's an absolute path. */
		new = malloc(len + 1);
		if (new == NULL)
			p_exit(PEXIT_FAILURE);
		strcpy(new, domain);
		strcat(new, url);
		safe_free(url);
	} else {		/* It's a path relative to the cwd. */
		new = malloc(len + strlen(path) + 1);
		if (new == NULL)
			p_exit(PEXIT_FAILURE);
		strcpy(new, domain);
		strcat(new, path);
		strcat(new, url);
		safe_free(url);
	}

	return new;
}

/*
	get_domain
	returns the domain based on the provided string.
	The returned string needs to be free()d.

	Expects an RFC 1738-style URL of the form:
		<scheme>://<user>:<password>@<host>:<port>/<url-path>
	See sections 2.1 and 3.1.
	
	Works by finding the first slash after the third character after the
	first colon, e.g.:
	'http://www.google.com/search?q=pr0n'
	                      ^
	                      right there.
	That example would return "http://www.google.com"; no trailing slash.
*/
static char *get_domain(const char url[])
{
	int end, len;
	char *r = NULL;

	assert(url != NULL);
	len = strlen(url);

	for (end = 0; end < len; end++)
		if (url[end] == ':')
			break;
	end += 3;
	for (; end < len; end++)
		if (url[end] == '/')
			break;

	safe_malloc(r,end + 1);
	strncpy(r, url, end);
	r[end] = '\0';
	return r;
}

static char *get_path(const char url[])
{
	int start, last, len;
	char *r = NULL;

	assert(url != NULL);
	len = strlen(url);
	
	for (start = 0; start < len; start++)
		if (url[start] == ':')
			break;
	start += 3;
	for (; start < len; start++)
		if (url[start] == '/')
			break;

	last = len - 1;
	while (url[last] != '/')
		last--;

	safe_malloc(r, last - start + 2);
	strncpy(r, url + start, last - start + 1);
	r[last - start + 1] = '\0';
	return r;
}

/*
	append_match
	Given a match, this function appends it to the provided list.  Designed
	to be used with regex_iterate.
	args:
		match = a regmatch_t describing where the match in s is.
		s = a string containing the match.
		max = Unused.	(maybe strip; check regex_iterate)
		list = array of strings.  void because regex_iterate calls this
	Returns nothing.
*/
void append_match(regmatch_t match, char *s, int max, void **list)
{
	int len = match.rm_eo - match.rm_so + 1, i;

	for (i = 0; list[i] != NULL; i++) {
		if (i >= max)
			return;	/* I can't take any more! */
		if (!strncmp(list[i], s + match.rm_so, len - 1))
			return;	/* add no duplicates */
	}

	safe_malloc(list[i],len);
	strncpy(list[i], s + match.rm_so, len - 1);
	((char *) list[i])[len - 1] = '\0';
}


/**
	url_cut
	Given a line, it finds all of the URLs needed to complete the page.
	args:
		line = a null-terminated string to do the matching inside.
		list = an array of regmatch_t's, allocated by the caller.
		max = the maximum number of matches and therefore the maximum
			number of elements that can be stored in list.
	returns the number of matches found
*/
int url_cut(char *line, regmatch_t list[], int max)
{
	regex_t src;
	int r, lindex = 0, lsave, loffset = 0;

	r = regcomp(&src, URL_REGEX, REG_EXTENDED | REG_ICASE);
	if (r) {
		io_err("Bad regex!!  Programmer error!!\n");
		abort();
	}

	memset(list, -1, max * sizeof(regmatch_t));

	while (!regexec(&src, line + lindex, 1, list + loffset, 0)) {
		lsave = lindex;
		lindex += list[loffset].rm_eo;
		list[loffset].rm_so += lsave;
		list[loffset].rm_eo += lsave;
		loffset++;
		if (loffset >= max)
			break;
	}

	regfree(&src);

	urls_only(line, list, max);
	return loffset;
}


/*
	urls_only
	takes a line, a list of matches, and the maximum number of matches,
	and removes the non-URL parts of the matches.

	this could use some cleaning up...it contains the deepest indentation
	in all of the code i've written for the probe.  (5 levels!)
*/
static void urls_only(char line[], regmatch_t list[], int max)
{
	int i, illegal = 1;
	for (i = 0; i < max; i++) {
		if (list[i].rm_so > 0) {
			list[i].rm_so = strchr(&line[list[i].rm_so], '=') + 
				1 - line;  /* skip over 'src=' */
			/*  Skip any illegal characters */
			illegal = 1;
			while (illegal == 1) {
				illegal = 0;
				switch (line[list[i].rm_so]) {
				case '=':
				case '\'':
				case '"':
					list[i].rm_so++;
					illegal = 1;
					break;
				}
				switch (line[list[i].rm_eo - 1]) {
				case '=':
				case '\'':
				case '"':
					list[i].rm_eo--;
					illegal = 1;
					break;
				}
			}
		}
	}
}
