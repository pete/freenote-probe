/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _URL_EXTRACT_H
#define _URL_EXTRACT_H

#include "probe.h"

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#define MAX_URL_LENGTH		1024
#define MAX_URLS_PER_LINE	32
#define MAX_TOTAL_URLS		32
#define URL_REGEX 	"([ \t]src=[\"']?[[.-.]\\&=\\?:a-z0-9\\._\\/]+[\"']?)"

int url_cut(char line[], regmatch_t list[], int max);
int extract_urls(char **urls, int maxurls, char *file, const char url[]);

#endif /* _URL_EXTRACT_H not defined? */
