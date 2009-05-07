#include "p_sysinfo.h"

#include <sys/utsname.h>

#include "err.h"

int os_name(char *buf)
{
	struct utsname un;
	int r = uname(&un);

	try(r);

	strcpy(buf, un.sysname);
	strcat(buf, " ");
	strcat(buf, un.release);
	strcat(buf, " ");
	strcat(buf, un.machine);
	strcat(buf, " (");
	strcat(buf, un.version);
	strcat(buf, ")");
	
	return 0;
}
