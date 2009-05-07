/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */


#ifndef	_FILEMAN_H
#define	_FILEMAN_H

int fm_init_base (void);

char *fm_read (char *);

/** write size bytes of buf to filename after prefixing it with base-dir */
int fm_write (char *filename, const char *buf, long size);

/** returns the size of a filename after prefixing it with base-dir */
long fm_size (char *);

/** prepends base-dir (".freenote") to filename, returns
 * a strdup'd string which needs to be free'd */
char *fm_abs (const char *);

/** fm_exists returns 1 if a file exists, 0 otherwise. */
int fm_exists(const char *file);

/** returns 1 if filename is a dir, 0 if not */
int fm_is_dir (const char *);

/**
 * Removes a file.  In the future, it will take Unix-style options
 * in "opts", but right now, it's okay to pass whatever.
 * Returns 0 on success, -1 on failure.
 */
int fm_rm(char *fname, char *opts);
int fm_team_info(char *team_name, char *pin_code);


/**
	fm_create_pidfile
	Creates a pid file.
*/
int fm_create_pidfile();
/**
	fm_remove_pidfile
	Removes a pid file.
*/
void fm_remove_pidfile();

#define FM_ID_PROBE                        "probe.id"
#define FM_KEY_PROBE_PUBLIC                "probe.pub"
#define FM_KEY_PROBE_PRIVATE               "probe.priv"
#define FM_KEY_DISPATCH_PUBLIC             "dispatch.pub"
#define FM_TEAM_INFO                       "probe.team"
#define FM_LOG_ERR                         "probe.error.log"
#define FM_LOG_OUT                         "probe.output.log"

#endif
