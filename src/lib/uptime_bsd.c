/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/* tested on openbsd 2.6, freebsd 5.3 -- also works on macos X */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/tty.h>

#include <machine/cpu.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <netdb.h>
#include <nlist.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <utmp.h>
#include <vis.h>

#include "io.h"

struct timeval	boottime;
struct utmp	utmp;
struct winsize	ws;
kvm_t	       *kd;
time_t		uptime;		/* time of last reboot & elapsed time since */
int		ttywidth;	/* width of tty */
int		argwidth;	/* width of tty */
int		header = 1;	/* true if -h flag: don't print heading */
int		nflag = 1;	/* true if -n flag: don't convert addrs */
int		sortidle;	/* sort bu idle time */
char	       *sel_user;	/* login of particular user selected */
char		domain[MAXHOSTNAMELEN];

#define	NAME_WIDTH	8
#define HOST_WIDTH	16

/*
 * One of these per active utmp entry.
 */
struct	entry {
	struct	entry *next;
	struct	utmp utmp;
	dev_t	tdev;			/* dev_t of terminal */
	time_t	idle;			/* idle time of terminal in seconds */
	struct	kinfo_proc2 *kp;	/* `most interesting' proc */
} *ep, *ehead = NULL, **nextp = &ehead;

static time_t	 get_uptime(void);

int
main(int argc, char *argv[])
{
	time_t uptime;

	uptime = get_uptime();

	io_debug("%i\n", uptime);
}


time_t get_uptime(void)
{
	/* this code is from openbsd src */

	double avenrun[3];
	time_t uptime, now;
	int days, hrs, i, mins;
	int mib[2];
	size_t size;
	char buf[256];

	now = time(NULL);

	/*
	 * Print how long system has been up.
	 * (Found by looking getting "boottime" from the kernel)
	 */
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) != -1) {
		uptime = now - boottime.tv_sec;
		return(uptime);
	} else {
		return(0);
	}
}

