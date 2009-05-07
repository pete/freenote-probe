/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */


#ifndef	_PROTO_H
#define	_PROTO_H

#ifndef WIN32
#include <sys/types.h>
#endif /* !WIN32 */

#include "probe.h"
#include "lib/nv_pair.h"
#include "p_types.h" 

#define	FN_DISPATCH		0
#define FN_COLLECTION		1

#define	FN_TYPE_CHECKIN		0x0001
#define FN_TYPE_GIVEWORK	0x0002
#define FN_TYPE_REPORT		0x0003
#define FN_TYPE_ASSIGN		0x0101
#define FN_TYPE_WORK		0x0102
#define FN_TYPE_SAVESUCCESS	0x0103
#define FN_TYPE_PING		0x0111
#define FN_TYPE_ERROR		0x01FF

typedef enum {
	PE_UNRECOGNIZED,

	PE_PIN_DENIED,

	PE_PROBEID_TOOSOON,
	PE_PROBEID_MISSING,
	PE_PROBEID_INACTIVE,
	PE_PROBEID_NOTFOUND,

	PE_WORKID_MISSING,
	PE_WORKID_EMPTY,
	PE_WORKID_INCOMPLETE,
	PE_WORKID_NOTFOUND,
	PE_WORKID_COMPLETE,
	PE_WORKID_DBERROR,

	PE_TEAMNAME_NOTFOUND,
	PE_TEAMNAME_INACTIVE,
} proto_err_t;

struct proto_header_t { 
	uint16	type;

	uint32	machine_id;

	uint16	code_version;
	uint16	protocol_version;
	uint16	size;
	
	char	signature[48];

} __attribute__ ((packed));

struct proto_mesg_t {

	struct proto_header_t	header;
	char			*body;
	size_t 			size;
};

/* build a protocol message */
int proto_mesg_build (struct proto_mesg_t *pm, int type, int id, nv_list *body);int proto_mesg_destroy (struct proto_mesg_t *pm);

/* send a protocol message to a collection or dispatch server */
int proto_mesg_send (struct proto_mesg_t *pm, struct proto_mesg_t *pmr,
		int server_type);

char *proto_get_chunk (char *buf, size_t *new_len);
nv_list * proto_get_nvl (struct proto_mesg_t *pm);

char *proto_mesg_encode (char *decoded, size_t len, size_t *new_len);
char *proto_mesg_decode (char *encoded, size_t *new_len);

int proto_error(struct proto_mesg_t pm);

void proto_dump(struct proto_mesg_t *pm);

#endif
