/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <sys/types.h>

size_t __fn_strnlen(const char *str, size_t max)
{
	size_t ret = 0;
	if (!str)
		return 0;
	while ((ret < max) && (*str != '\0')) {
		++ret;
		++str;
	}
	return ret;
}

