/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "lib/collection.h"
#include "lib/nv_pair.h"
#include "lib/task.h"
#include "lib/fileman.h"
#include "lib/proto.h"
#include "lib/probecast.h"
#include "crypt/dsa/pdsa.h"

#include "urlencode.h"
#include "util.h"
#include "err.h"
#include "options.h"
#include "io.h"

/* *** */

int collection_init(void);
int collection_report(void);

static int probe_id = -1;

/* *** */

int send_results(const char *server)
{
	if (collection_init()) { return -1; };
	return(collection_report());
}


int collection_init(void)
{
	char	*tmp;

	if(probe_id != -1)
		return 0;
	
	tmp = fm_read(FM_ID_PROBE);

	if (tmp == NULL) {
		io_err("%s:%s():%d:  could not read FM_ID_PROBE\n", 
			__FILE__, __func__, __LINE__);
		return(-1);
	} else {
		sscanf(tmp, "%i", &probe_id);
		safe_free(tmp);
	}

	io_info("probe_id: %i\n", probe_id);
	return(0);
}

int collection_report(void)
{
	struct proto_mesg_t pm, pmr;
	int	rv = 0, gcv, widg = 0;
	nv_list	*nvl;
	task_t	t;
	char	buf[256], wid[INT_TO_S_SIZE], tid[INT_TO_S_SIZE];
	
	nvl = nv_list_init(MAX_TASKS + 1);

	probecasts("Status:  Reporting results.\n");

	while ((gcv = get_completed(&t)) != -1) {
		if (gcv == 0 || gcv == 1) {
			task_to_s(buf, t);
			if (!widg) { sprintf(wid, "%i", t.workid); ++widg; } 
			sprintf(tid, "%i", t.taskid);
			nv_list_insert(nvl, tid, buf, 0);
		}
	}

	nv_list_insert(nvl, "work_id", wid, 0);

	nv_list_dump(nvl);
	
	try(proto_mesg_build(&pm, FN_TYPE_REPORT, probe_id, nvl));
	gcv = proto_mesg_send(&pm, &pmr, FN_DISPATCH);
	while(gcv == -1) {
		print_loc();
		io_err(	"Trouble sending results back.  Re-trying in %d "
			"seconds.\n", read_int_opt("interval"));
		sleep(read_int_opt("interval"));
		gcv = proto_mesg_send(&pm, &pmr, FN_DISPATCH);
	}
		

	nv_list_destroy(nvl);

	if(!valid_dsa_sig(pmr.body, pmr.size, pmr.header.signature)) {
		proto_mesg_destroy(&pm);
		proto_mesg_destroy(&pmr);
		io_err(	"ERROR:  %s returned data with a bad signature!\n"
			"This may indicate a man-in-the-middle attack.\n"
			"Aborting!\n",
			read_str_opt("collection-server"));
		return -1;
	}

	if(pmr.header.type == FN_TYPE_ERROR) { 
		io_warning("The collection server (%s) returned an error!\n",
			read_str_opt("collection-server"));
		proto_dump(&pm);
		proto_dump(&pmr);
		rv = -(proto_error(pmr) == -1);
	} else {
		nvl = proto_get_nvl(&pmr);

		io_debug("REPORT:\n");
		proto_dump(&pm);
		proto_dump(&pmr);
	
		nv_list_dump(nvl);
		nv_list_destroy(nvl);
	}
		
	proto_mesg_destroy(&pm);
	proto_mesg_destroy(&pmr);

	return(rv);
}


