/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#include "lib/cookie.h"

static int extract_cookie(cookie *c, char * const cs);

/**
	init_cookie_list
	Pass an array of cookies and the size of the array.  We shall
	initialize them!  
*/
void init_cookie_list(cookie clist[], int n)
{
	int i;
	for(i = 0; i < n; i++)
		clist[i].magic_cookie = ~COOKIE_MAGIC;
}

/**
	parse_cstring
	Provided a place to put the cookie list, a maximum number of cookies to
	extract, and a string with the values in it, parse_cstring() stuffs up 
	to cmax cookies into the array.  We return the number of cookies pulled
	out of the string.

	Right now, since almost all web browsers and servers use v1 of the 
	cookie standard, and there is no standard for v2, which no one uses
	anyway, we're just doing v1.  
*/
int parse_cstring(cookie clist[], int cmax,  char * const cstring)
{
	int	n;	
	char	*current = cstring;
	
	for(n = 0; n < cmax; n++) {
		current = strstr(current, "Set-Cookie: ");
		if(current == NULL)
			break;
		current += 13;	/* strlen("Set-Cookie: ") = 13 */
		n += extract_cookie(&clist[n], current);
	}

	return n;
}


/**
	write_cookies
	writes all the valid cookies in the array, up to the maximum
	Returns 0 - (number of cookies not written)
*/
int write_cookies(const int fd, cookie c[], int max)
{
	int i, n, total = 0;
	write(fd, "Cookie: ", 8);
	for(i = max; i-- ; ) {
		total += (n = write_cookie(fd, &c[i]));
		if(n != -1)	
			write(fd, "; ", 2);
	}
	write(fd, "\r\n", 2);
	return total;
}

/**
	write_cookie
	Given a cookie and a file descriptor, write_cookie() writes the cookie
	to the file descriptor.  WOWZERS~!!

	Returns 0 for success, -1 for incomplete or bad write.
*/
int write_cookie(const int fd, cookie *c)
{
	int i, total;
	if(!valid_cookie(c))
		return -1;
	total = strlen(c->content);
	i = write(fd, c->content, total);
	if(i == total)
		return 0;
	return -1;
}

/**
	valid_cookie
	given a cookie pointer, we check if it's valid and return 1 if it is, 
	0 if it's not.  
	We don't modify the cookie, it's a performance thing.  Passing a 
	pointer is easier on the stack than passing the just-over-3k cookie.
*/
int valid_cookie(const cookie *c)
{
	return (c->magic_cookie == COOKIE_MAGIC && c->content != NULL);
}

/**
	cookies_to_s
	Converts a list of cookies (up to max) to a string and stores it in 
	s.  s must point to at least cookies_strlen(cookies, max) bytes of
	memory.  Returns a pointer to the string, which ends up looking
	something like this:
Cookie: Name1=Value1; Name2=Value2; ...\r\n

	If you plan to dynamically allocate the memory, you might want to look
	at cookies_strdup().
*/
char *cookies_to_s(char *s, cookie cookies[], int max)
{
	int 	i;
	
	strcpy(s, "Cookie: ");
	for(i = 0; i < max; i++) 
		if(valid_cookie(&cookies[i])) {
			strcat(s, cookies[i].content);
			strcat(s, "; ");
		}
	strcat(s, "\r\n");
	return s;
}

/**
	cookies_strlen
	Given a list of cookies and the maximum number of cookies in the list,
	cookies_strlen returns the amount of memory you'll have to allocate to
	hold the cookie string, which is how much memory needs to be on the
	other side of the 'buf' argument for cookies_to_s.
*/
int cookies_strlen(cookie cookies[], int max)
{
	int	i,
		size = 0;
	for(i = 0; i < max; i++)
		if(valid_cookie(&cookies[i]))
			size += strlen(cookies[i].content) + 2;	//"; "
	if(!size)
		return 0;
	
	return (size + 11); /*strlen("Cookie: ") + strlen("\r\n\0")*/
}

/**
	cookies_strdup
	Returns a dynamically allocated string you can pass straight to the
	webserver and then free().  You pass a list of cookies and the max
	cookies in the list.
*/
char *cookies_strdup(cookie cookies[], int max)
{
	char *s = NULL;
	int len = cookies_strlen(cookies, max);
	if (len == 0)
		return NULL;

	safe_malloc(s, len);

	return cookies_to_s(s, cookies, max);
}

#define copy_property(x, l) do {					\
			c->x[0] = '\0';					\
			if(x != NULL) {					\
				x += l;					\
				semicolon = strchr(x, ';');		\
				if(semicolon != NULL) 			\
					strncpy(c->x, x, semicolon - x);\
			}						\
			} while(0)
static int extract_cookie(cookie *c, char * const cs)
{
	char	*semicolon = strchr(cs, ';'),
		*domain = strstr(cs, "domain="),
		*expires = strstr(cs, "expires="),
		*path = strstr(cs, "path=");

	if(semicolon == NULL)
		return -1;
	strncpy(c->content, cs, semicolon - cs);

	copy_property(domain, 7);
	copy_property(expires, 8);
	copy_property(path, 5);
	c->magic_cookie = COOKIE_MAGIC;
	return 0;
}
#undef copy_property
