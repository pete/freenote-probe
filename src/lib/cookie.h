/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef _COOKIE_H
#define _COOKIE_H

#include <time.h>

#define MAX_COOKIE_SIZE 1024	/* This seems generous */
#define MAX_COOKIE_PATH 1024	/* Max length for a URL is 1024 */
#define MAX_COOKIE_DOMAIN 1024	/* Max length for a URL is 1024 */
#define MAX_COOKIE_DATE 32	/* `date -R | wc -c` (minus '\n' plus '\0')*/

#define COOKIE_MAGIC 0x63636363  /* Cookie, cookie, cookie starts with 'C'!*/
	

typedef struct {
	int magic_cookie;
	char content[MAX_COOKIE_SIZE];
	char path[MAX_COOKIE_PATH];
	char domain[MAX_COOKIE_DOMAIN];
	char expires[MAX_COOKIE_DATE];
} cookie;
	
void init_cookie_list(cookie clist[], int n);
int parse_cstring(cookie clist[], int cmax, char * const cstring);
int write_cookies(const int fd, cookie c[], int max);
int write_cookie(const int fd, cookie *c);
char *cookies_to_s(char *s, cookie cookies[], int max);
int cookies_strlen(cookie cookies[], int max);
char *cookies_strdup(cookie cookies[], int max);
int valid_cookie(const cookie *c);

#endif /* _COOKIE_H defined? */
