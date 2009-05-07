/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <time.h>

#include "lib/dispatch.h"
#include "lib/nv_pair.h"
#include "lib/task.h"
#include "lib/fileman.h"
#include "lib/proto.h"
#include "lib/probecast.h"
#include "crypt/dsa/pdsa.h"
#include "lib/p_sysinfo.h"

#include "urlencode.h"
#include "util.h"
#include "err.h"

int dispatch_init(void);
int dispatch_checkin(void);
int dispatch_getwork();

static struct {
	int probe_id;	//Static globals are implicitly initialized to zero.
	char team_name[1024];
	char pin_code[1024];
} probe_info;

int get_jobs(const char *server)
{
	try(dispatch_init());
	try(dispatch_getwork());
	return 0;
}

int dispatch_init(void)
{
	int size;
	char *tmp;
	char id_to_s[INT_TO_S_SIZE + 28];

	if(probe_info.probe_id != 0)
		return 0;

	if(fm_team_info(probe_info.team_name, probe_info.pin_code) == -1) {
		io_warning("No team info file found in .freenote!\n"
			"Please visit freenote.petta-tech.com and register"
			" to join or start a team.\n");
		strcpy(probe_info.team_name, "default");
		strcpy(probe_info.pin_code, "1234");
	}	

	tmp = fm_read(FM_ID_PROBE);

	if (tmp == NULL) {
		dispatch_checkin();
	} else {
		sscanf(tmp, "%i", &probe_info.probe_id);
		safe_free(tmp);
	}
	sprintf(id_to_s, "Probeid:  %u\nStatus:  on-line!\n", 
		probe_info.probe_id);
	probecasts(id_to_s);

	io_info("probe_id: %i\n", probe_info.probe_id);
	return (0);
}

int dispatch_checkin(void)
{
	struct proto_mesg_t pm, pmr;
	char *ppk = NULL, *ppke;
	char os_desc[OS_NAME_SIZE];
	const char *tmp; 
	int rv = 0;
	nv_list *nvl;

	ppk = dsa_get_ppk();
	ppke = urlencode_dup(ppk, 0);
	safe_free(ppk);

	os_name(os_desc);

	nvl = nv_list_init(4);
	nv_list_insert(nvl, "team_name", probe_info.team_name, 0);
	nv_list_insert(nvl, "pin_code", probe_info.pin_code, 0);
	nv_list_insert(nvl, "os_type", os_desc, 0);
	nv_list_insert(nvl, "pub_key", ppke, 0);
	safe_free(ppke);

	try(proto_mesg_build(&pm, FN_TYPE_CHECKIN, 0, nvl));
	try(proto_mesg_send(&pm, &pmr, FN_DISPATCH));

	nv_list_destroy(nvl);

	if (pmr.header.type == FN_TYPE_ERROR) {
		//proto_mesg_destroy(&pm);
		//proto_mesg_destroy(&pmr);
		rv = -(proto_error(pmr) == -1);
	}

	nvl = proto_get_nvl(&pmr);

	io_debug("CHECKIN\n");
	proto_dump(&pm);
	proto_dump(&pmr);

	nv_list_dump(nvl);

	tmp = nv_hash(nvl, "probe_id");
	if(tmp == NULL) {
		io_err("Null probe_id!\n");
		rv = -1;
	} else {
		int written;
		probe_info.probe_id = atoi(tmp);
		written = fm_write(FM_ID_PROBE, tmp, strlen(tmp));
		if (written <= 0) {
			io_err("failed to write %s to %s!\n",tmp,FM_ID_PROBE);
			rv = -1;
		}
	}
	tmp = nv_hash(nvl, "dispatch_key");
	if(tmp == NULL) {
		io_err("The dispatch server gave us an empty public key!\n");
		rv = -1;
	} else if (rv != -1) {
		rv = save_dispatch_key(tmp);
	}

	nv_list_destroy(nvl);

	proto_mesg_destroy(&pm);
	proto_mesg_destroy(&pmr);

	return (rv);
}

//SEGV:  There's a seg fault somewhere in here.
int dispatch_getwork()
{
	struct proto_mesg_t pm, pmr;
	char scratch[INT_TO_S_SIZE + 1];
	char id_to_s[INT_TO_S_SIZE + 37];
	int i, rv = 0;
	int work_id;
	const char *wh;
	nv_list *nvl;
	task_t t;

	/* must have a valid DSA public key in opt(base-dir)
	   named probe.pub */

	sprintf(id_to_s, "Probeid:  %u\nStatus:  Getting work...\n", 
		probe_info.probe_id);
	probecasts(id_to_s);

	nvl = nv_list_init(2);
	if(nvl == NULL) {
		print_loc();
		io_err("Out of memory!\n");
		return -1;
	}

	sprintf(scratch, "%i", (int) time(NULL));
	nv_list_insert(nvl, "time", scratch, 0);
	caps_to_s(scratch);
	nv_list_insert(nvl, "capabilites", scratch, 0);

	i = proto_mesg_build(&pm, FN_TYPE_GIVEWORK, probe_info.probe_id, nvl);
print_loc(); io_debug("\n<<<<<<<<<<<<<<<pmr->body:  0x%x\n", pmr.body);
	nv_list_destroy(nvl);
	if(i < 0) {
		print_loc(); io_debug("Couldn't build protocol message!\n");
		return -1;
	}
	try(proto_mesg_send(&pm, &pmr, FN_DISPATCH));


print_loc(); io_debug("\n<<<<<<<<<<<<<<<pmr->body:  0x%x\n", pmr.body);
	if (pmr.header.type == FN_TYPE_ERROR) {
		rv = -(proto_error(pmr) == -1);
	}
print_loc(); io_debug("\n<<<<<<<<<<<<<<<pmr->body:  0x%x\n", pmr.body);

	nvl = proto_get_nvl(&pmr);
print_loc(); io_debug("\n<<<<<<<<<<<<<<<pmr->body:  0x%x\n", pmr.body);

	io_debug("GIVEWORK:\n");
	proto_dump(&pm);
	proto_dump(&pmr);
	proto_mesg_destroy(&pm);
	proto_mesg_destroy(&pmr);

	nv_list_dump(nvl);

	wh = nv_hash(nvl, "work_id");
	if(!(wh && (work_id = atoi(wh)))) {
		print_loc();
 		io_err("Didn't get work_id!\n");
		return -1;
	}

	for (i = 0; i < nvl->size; ++i) {
		if (nvl->nvp[i] != NULL &&
		    strcmp(nvl->nvp[i]->name, "work_id")) {
			t = nv_to_task(work_id, nvl->nvp[i]->name,
				       nvl->nvp[i]->value);
			if (valid_task(t)) {
				task_add(t);
			} else
				io_err("Could not add task!\n");

		}
	}
	nv_list_destroy(nvl);

	return (rv);
}

