#ifndef _P_SYSINFO_H
#define _P_SYSINFO_H

#include <sys/utsname.h>

#define OS_NAME_SIZE sizeof(struct utsname)	//Can't be any bigger than that.

/**
	Copies a nicely formatted string describing the OS into the buf, which
	should be at least OS_NAME_SIZE characters long.
*/
int os_name(char *buf);

#endif /* _P_SYSINFO_H */
