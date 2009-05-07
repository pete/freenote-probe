/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "probe.h"
#include "options.h"
#include "util.h"
#include "err.h"
#include "lib/fileman.h"

int fm_init_base (void)
{
	int rv = 0;
	char *base_dir = read_file_opt_dup("base-dir");

	if (!fm_is_dir(base_dir)) {
		if (mkdir(base_dir, S_IRWXU) == -1) {
			io_err("unable to create directory: %s\n", 
				base_dir);
			rv = -1;
		}
	}
	safe_free(base_dir);

	return rv;
}

/** converts a text file -> string  Returns NULL on error.  mallocs a and
 *  returns a new string on success.  Remember to free this afterwards! */
char *fm_read (char *filename)
{
	FILE	*fh;
	char	*buf = NULL, *fa = NULL;
	long	fsize;

	fa = fm_abs(filename);

	fsize = fm_size(filename);
	
	if (!fsize)
		goto end;
	
	fh = fopen(fa, "r");

	safe_malloc(buf, fsize + 1);

	if (!fread(buf, fsize, 1, fh))
		buf = NULL;
	else
		buf[fsize] = '\0';

	fclose (fh);
end:
	safe_free(fa);
	return(buf);
}

/** 
	fm_write
	write size bytes of buf to filename after prefixing it with base-dir 
	returns 0 for failure, 1 for success.
*/
int fm_write (char *filename, const char *buf, long size)
{
	FILE	*fh;
	char	*fa = NULL;
	int	rv;

	fa = fm_abs(filename);

	/* TODO: set permissions */
	if (!(fh = fopen(fa, "w"))) return(0);

	rv = fwrite(buf, size, 1, fh);

	fclose (fh);

	safe_free(fa);
	return(rv);
}

/** returns the size of a filename after prefixing it with base-dir */
long fm_size (char *filename)
{
	char	*fa = NULL;
	struct stat s;

	fa = fm_abs(filename);

	if(stat(fa, &s) == -1)
		s.st_size = 0;

	safe_free(fa);

	return (s.st_size);
}



/** prepends base-dir ("~/.freenote") to filename, returns
 * a strdup'd string which needs to be free'd */
char *fm_abs (const char *filename) 
{
	char *tmp = NULL, *base_dir = NULL;
	assert(filename != NULL);
	
	base_dir = read_file_opt_dup("base-dir");
	
	safe_malloc(tmp, strlen(base_dir) + strlen(filename) + 2);
	
	strcpy(tmp, base_dir);
	strcat(tmp, "/");
	strcat(tmp, filename);

	safe_free(base_dir);
	return(tmp);
}

/** 
	fm_exists
	Tests to see whether the file specified exists in the base-dir or not.  
	Returns 0 for false, 1 for true.
*/
int fm_exists(const char *file)
{
	struct stat s;
	int rv;
	char *fn = fm_abs(file);

	rv = stat(fn, &s);
	free(fn);

	return rv + 1;
}

/** returns 1 if filename is a dir, 0 if not */
int fm_is_dir (const char *filename)
{
	struct stat	sb;

	if (!stat(filename, &sb)) {
		if (S_ISDIR(sb.st_mode)) { return(1); }
	}
	
	return(0);
}

/**
	fm_rm
	Removes a file.  In the future, it will take Unix-style options
	in "opts", but right now, it's okay to pass whatever.
	Returns 0 on success, -1 on failure.
*/
int fm_rm(char *fname, char *opts)
{
	int r;
	char *fpath = fm_abs(fname);

	if(fpath == NULL)
		return -1;

	r = unlink(fpath);

	safe_free(fpath);
	return r;
}

/**
	fm_team_info
	Reads the team name and pin code into the two strings passed to it.
	Returns 0 for success, -1 for error.
*/
int fm_team_info(char *team_name, char *pin_code)
{
	char	*tmp,
		*end,
		*data;

	data = fm_read(FM_TEAM_INFO);
	if(data == NULL)
		return -1;

	// I'd like to clean this up, maybe make a generalized function (or
	// possibly a macro) to handle the repetition.  FIXME
	tmp = strstr(data, "team_name:");
	if(tmp == NULL) {
		safe_free(data);
		return -1;
	}
	tmp += 10;
	end = strchr(tmp, ';');
	if(end == NULL) {
		safe_free(data);
		return -1;
	}
	memcpy(team_name, tmp, end - tmp);
	team_name[end - tmp + 1] = '\0';
	tmp = strstr(data, "pin_code:");
	if(tmp == NULL) {
		safe_free(data);
		return -1;
	}
	tmp += 9;
	end = strchr(tmp, ';');
	if(end == NULL) {
		safe_free(data);
		return -1;
	}
	memcpy(pin_code, tmp, end - tmp);
	pin_code[end - tmp + 1] = '\0';
	
	safe_free(data);
	return 0;
}

int fm_create_pidfile()
{
#ifndef CYGWIN32
	char * file = read_file_opt_dup("pid-file");
	FILE * pf;
	/* Not using fm_abs because it's restricted to base-dir. */
	if(file == NULL)
		return PEXIT_FAILURE;
	pf = fopen(file,"w");
	if (pf == NULL) {
		print_loc();
		io_err("Failed to open pid-file '%s'.\n",file);
		safe_free(file);
		return PEXIT_FAILURE;
	}
	safe_free(file);
	fprintf(pf,"%d\n",getpid());
	fclose(pf);
#endif /* CYGWIN32 */
	return 0;
}

void fm_remove_pidfile()
{
#ifndef CYGWIN32
	char * file = read_file_opt_dup("pid-file");
	if (file != NULL)
		unlink(file);
	safe_free(file);
#endif /* CYGWIN32 */
}
