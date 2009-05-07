/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

#include "probe.h"
#include "sys/unix.h"
#include "io.h"
#include "err.h"

static int chuser(const char *user)
{
	struct passwd *pw;
	char pcs[1024];
	int uid = atoi(user), gid = 0;

	unless(uid) {
		pw = getpwnam(user);
		if(pw == NULL)
			return -1;
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	} else {
		print_loc(); io_warning("Could not get info for user %s.  "
					"Not trying to su.\n", user);
		return -1;
	}

	if(gid && setgid(gid)) {
		print_loc(); io_warning("Could not setgid to %d.\n", gid);
	}
	if(setuid(uid)) {
		print_loc(); 
		io_warning("Could not change uid to %s(%d)!\n", user, uid);
		return -1;
	}

	return 0;
}


/**
	Have the current process run in the background, as 'nobody' if possible.
*/
void unix_daemonize()
{
	pid_t pid;
	fflush(NULL);
	pid = fork();

	if(pid < 0) {
		print_loc();
		io_err("Could not fork()!\n");
		p_exit(PEXIT_FAILURE);
	} else if(pid > 0) {
		_exit(EXIT_SUCCESS);
	}

	if(chdir("/") < 0) {
		/*strerror(errno); */
		p_exit(PEXIT_FAILURE);
	}

	if(!getuid())
		chuser(read_str_opt("su-user"));

	if(setsid() < 0) {
		/* error message */
		p_exit(PEXIT_FAILURE);
	}

	umask(0066);

	fflush(NULL);
	pid = fork();
	if(pid < 0) {
		/* error message */
		p_exit(PEXIT_FAILURE);
	} else if(pid > 0) {
		_exit(EXIT_SUCCESS);
	}
}
