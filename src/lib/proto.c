/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <string.h>
#include "sys/compat.h"

#include <stdio.h>
#include <stdlib.h>

#include "crypt/dsa/pdsa.h"
#include "crypt/base64/base64.h"
#include "options.h"
#include "lib/proto.h"
#include "lib/fileman.h"
#include "lib/dispatch.h"
#include "net-modules/http.h"
#include "net-modules/dns.h"

#include <openssl/dsa.h>

#include "util.h"
#include "p_types.h"
#include "urlencode.h"
#include "err.h"
#include "lib/probecast.h"

static proto_err_t proto_errno(struct proto_mesg_t pm);

/*  We should be using htons, which always does The Right Thing, ensuring that
    the number is converted to network byte regardless of the host's endianness.
    We would need to change the protocol to expect network byte order instead of
    host byte order, which could be tricky.
*/
#ifdef WORDS_BIGENDIAN
static void __byteswap(unsigned char *ptr, size_t size)
{
	unsigned int i;
	for (i = 0; i < (size/2); ++i) {
		unsigned char t = ptr[i];
		ptr[i] = ptr[size - i - 1];
		ptr[size - i - 1] = t;
	}
}

# define byteswap(a) do { \
	__byteswap((unsigned char *)&(a),sizeof(a)); \
} while (0)
#else  /* !WORDS_BIGENDIAN */
# define byteswap(a)
#endif /* !WORDS_BIGENDIAN */

void proto_dump(struct proto_mesg_t *pm) 
{
	io_debug("\nheader.type\t\t0x%.4x\n", pm->header.type);
	io_debug("header.machine_id\t%i\n", pm->header.machine_id);
	io_debug("header.code_version\t%i\n", pm->header.code_version);
	io_debug("header.protocol_version\t%i\n", pm->header.protocol_version);
	io_debug("header.size\t\t%i\n", pm->header.size);

	io_debug("body: *");
	io_debug("%s",pm->body);
	io_debug("*\n\n");                     	
}

/**
	proto_mesg_build
	Creates a proto_mesg_t (pm) from an nv_list (nvl).  type should be the
	type of message (see proto.h) and id should be the probe's id.
	Returns 0 for success, -1 for problems.
*/
int proto_mesg_build (struct proto_mesg_t *pm, int type, int id, nv_list *nvl)
{
	char	*body = NULL;
	size_t	body_size;

	pm->body = NULL;
	
	pm->header.type = htons(type);
	pm->header.machine_id = id;
	pm->header.code_version = VERSION16BIT;
	pm->header.protocol_version = PROTOCOL_VERSION; 
	
	byteswap(pm->header.code_version);
	byteswap(pm->header.protocol_version);
	byteswap(pm->header.machine_id);

	body = nv_list_to_string(nvl);
	body_size = strlen(body);

	pm->header.size = (uint16)body_size;

	safe_malloc(pm->body, body_size + 1);
	memcpy(pm->body, body, body_size + 1);
	safe_free(body);
	try(gen_dsa_sig(pm->body, body_size, pm->header.signature));

	return(0);
}

int proto_mesg_destroy (struct proto_mesg_t *pm)
{
	safe_free(pm->body);
	return(0);
}

int proto_mesg_send (struct proto_mesg_t *pm, struct proto_mesg_t *pmr,
		int server_type)
{
	char	*raw = NULL, *out = NULL;
	char    *ue = NULL,*encoded = NULL, *decoded = NULL;
	const char	*server = NULL;
	size_t	size, size_raw;
	http_request *req;
	struct in_addr addr;

	switch (server_type) {
	case FN_DISPATCH:
		server = read_str_opt("dispatch-server");
		break;
	case FN_COLLECTION:
		server = read_str_opt("collection-server");
		break;
	default:
		print_loc();
		io_err("Unknown server type!: %d\n", server_type);
		p_exit(PEXIT_FAILURE);
		break;
	}

	
	addr = resolve_cached(server, NULL, NULL);
	while(addr.s_addr == INADDR_NONE) {
		int interval = read_int_opt("interval");
		print_loc();
		io_err(	"Temporary failure in name resolution.  Retrying in "
			"%d seconds...\n", interval);
		sleep(interval);
		addr = resolve(server, NULL, NULL);
	}

	size_raw = sizeof(pm->header) + pm->header.size;
	safe_malloc(raw, size_raw);

	memcpy(raw, &(pm->header), sizeof(pm->header));
	memcpy(raw + sizeof(pm->header), pm->body, pm->header.size);

	/* base64 encode */
	encoded = proto_mesg_encode(raw, size_raw, &size);

	ue = urlencode_dup(encoded, 0);
	safe_free(encoded);
	size = strlen(ue);

	safe_free(raw);

	req = new_http_req(addr, 6060, server,
		"/apps/as_dispatch", 0, NULL, NULL);
	if(req == NULL) {
		print_loc();
		io_debug("Couldn't connect to server.\n");
		return -1;
	}

	req->version = 1;

	safe_malloc(out, size + 4);
	strcpy(out, "fn=");
	memcpy(out + 3, ue, size);
	safe_free(ue);

	http_post_str(req, out);
	
	safe_free(out);
	if (req->body == NULL) {
		print_loc();
		io_err("Failed to get a response!\n");
		http_req_destroy(req);
		return -1;
	}

	size = req->body_len;
	decoded = proto_mesg_decode(req->body, &size);

	http_req_destroy(req);
	
	if(decoded == NULL) {
		io_debug("%s:%s():%d:  decoded == NULL!\n", __FILE__, __func__,
			__LINE__);
		return -1;
	}
	
	memset(&pmr->header, 0, sizeof(pmr->header));
	memcpy(&pmr->header, decoded, sizeof(pmr->header));	
	size -= sizeof(pmr->header);

	pmr->header.type = htons(pmr->header.type);

	byteswap(pmr->header.code_version);
	byteswap(pmr->header.protocol_version);
	byteswap(pmr->header.machine_id);
	byteswap(pmr->header.size);

	pmr->body = NULL;
	/* add one to null terminate */
	safe_calloc(pmr->body, size + 1);
	memcpy(pmr->body, (decoded + sizeof(pmr->header)), size);
	pmr->size = size;

	safe_free(decoded);

	return(0);

}

nv_list * proto_get_nvl (struct proto_mesg_t *pm)
{
	return nv_urlstring_to_list(pm->body);
}

char *proto_mesg_encode (char *decoded, size_t len, size_t *new_len)
{

	BIO	*b64, *bmem;
	BUF_MEM	*bptr;
	char	*encoded = NULL;
	size_t	size;

	safe_malloc(encoded, 4096);

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	bmem = BIO_push(b64, bmem);
	BIO_write(bmem, decoded, len);
	BIO_flush(bmem);

	BIO_get_mem_ptr(bmem, &bptr);
	BIO_set_close(bmem, BIO_NOCLOSE);

	size = (size_t)bptr->length;

	memcpy(encoded, bptr->data, size);
	*new_len = size;

	if (bmem)
		BIO_free_all(bmem);
	
	return(encoded);
}

char *proto_mesg_decode (char *encoded, size_t *new_len)
{
	char * ret = b64decode(encoded, new_len);
	return ret;
}

/*
	proto_error
	Returns 0 for retry, 1 for no major problem and -1 for horrifying error.

	Retry Errors:
		probe_id TOOSOON  (sleeps when found)
	Fatal Errors:
		pin_code DENIED
		probe_id MISSING 
		probe_id INACTIVE
		team_name NOTFOUND
		team_name INACTIVE
		work_id MISSING
		work_id EMPTY
		work_id INCOMPLETE
	Warnings:
		work_id COMPLETE
		work_id NOTFOUND
	Special Cases:
		probe_id NOTFOUND:  checks in again and returns 1
*/
/*  Large and ugly by its nature  */
int proto_error(struct proto_mesg_t pm)
{
	switch(proto_errno(pm)) {
                case PE_PROBEID_TOOSOON:
			sleep(read_int_opt("interval"));
			return 0;

                case PE_PROBEID_NOTFOUND:
			fm_rm(FM_ID_PROBE, "");
			return dispatch_init();

		case PE_PIN_DENIED:
		case PE_PROBEID_INACTIVE:
		case PE_TEAMNAME_NOTFOUND:
		case PE_TEAMNAME_INACTIVE:
			print_loc();
			io_err(	"FATAL ERROR:  %d!\n"
				"This probe has been denied access!  Please "
				"check to make sure that your team\n"
				"name and pin code (in the probe.team file in "
				"your ~/.freenote directory) are\n"
				"valid.  You may also have been denied access "
				"because of illegal actions by\n"
				"the team or your probe.  If this is the case, "
				"your team's leader will receive\n"
				"an e-mail explaining the cause.\n"
				, proto_errno(pm));
			probecasts("FATAL:  Denied access!\n");
			p_exit(PEXIT_FAILURE);

		case PE_PROBEID_MISSING:
                case PE_WORKID_MISSING:
                case PE_WORKID_EMPTY:
			break;

		case PE_WORKID_DBERROR:
			io_err("Server-side database problems.\n");
			break;

                case PE_WORKID_INCOMPLETE:
			io_warning("Sent back an incomplete work packet!\n");
			return 0;
                case PE_WORKID_COMPLETE:
			io_warning("Reporting already-complete work packet!\n");
			return 0;
                case PE_WORKID_NOTFOUND:
			io_warning("Reporting an invalid work packet!\n");
			return 0;

                case PE_UNRECOGNIZED:
		default:
			print_loc();
			io_err("Unrecognized error from the server!\n");
			return -1;
	}
}

/*  Large and ugly, just like [Flame removed.  Pretend to be offended.]. */
static proto_err_t proto_errno(struct proto_mesg_t pm)
{
	nv_list *nvl;
	proto_err_t r = PE_UNRECOGNIZED;
	const char	*var,
			*code;
	
	nvl = proto_get_nvl(&pm);

	if(nvl == NULL) {
		print_loc();
		io_err("Internal error:  could not convert message to nvl!\n");
		return PE_UNRECOGNIZED;
	}

	var = nv_hash(nvl, "error_var.1");
	code = nv_hash(nvl, "error_code.1");

	if(!strcmp(var, "pin_code")) {			// pin_code errors
		if(!strcmp(code, "DENIED"))
			r = PE_PIN_DENIED;
	} else if(!strcmp(var, "probe_id")) {		// probe_id errors
		if(!strcmp(code, "NOTFOUND")) 
			r = PE_PROBEID_NOTFOUND;
		else if(!strcmp(code, "TOOSOON")) 
			r = PE_PROBEID_TOOSOON;
		else if(!strcmp(code, "MISSING"))
			r = PE_PROBEID_MISSING;
		else if(!strcmp(code, "INACTIVE"))
			r = PE_PROBEID_INACTIVE;
	} else if(!strcmp(var, "work_id")) {		// work_id errors
		if(!strcmp(code, "MISSING"))
			r = PE_WORKID_MISSING;
		else if(!strcmp(code, "EMPTY"))
			r = PE_WORKID_EMPTY;
		else if(!strcmp(code, "INCOMPLETE"))
			r = PE_WORKID_INCOMPLETE;
		else if(!strcmp(code, "NOTFOUND"))
			r = PE_WORKID_NOTFOUND;
		else if(!strcmp(code, "COMPLETE"))
			r = PE_WORKID_COMPLETE;
		else if(!strcmp(code, "DBERROR"))
			r = PE_WORKID_DBERROR;
	} else if(!strcmp(var, "team_name")) {		// team_name errors
		if(!strcmp(code, "NOTFOUND"))
			r = PE_TEAMNAME_NOTFOUND;
		else if(!strcmp(code, "INACTIVE"))
			r = PE_TEAMNAME_INACTIVE;
	}

	nv_list_destroy(nvl);
	return r;
}

