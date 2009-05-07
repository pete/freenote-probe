/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __LIB_NV_PAIR_H
#define __LIB_NV_PAIR_H

/** nv_list, handles arrays of arbitrary name=value pairs (must be text) */

typedef struct _nv_pair {
	char * name;
	void * value;
} nv_pair;

typedef struct _nv_list {
	nv_pair ** nvp;
	unsigned size;      /* number of name=value pairs housed */
	unsigned data_size; /* combined string lengths of all nv_pairs */
	unsigned last;      /* index of the last object inserted */
} nv_list;

/** initialize and allocate a new nv_list */
nv_list * nv_list_init (unsigned size);

/** free all memory used by an nv_list */
void nv_list_destroy (nv_list * nvl);

/** Insert a name=value pair into an nv_list, i is the index to insert
 * this as, if it its zero, then it'll insert into the next free slot */
unsigned nv_list_insert (nv_list * nvl, const char * name, const char * value,
		unsigned i);

/** dump an nv_list for debugging */
void nv_list_dump (nv_list * nvl);

/** Convert an nv_list into a string */
char * nv_list_to_string (const nv_list * nvl);

/** Convert a URL string into a new nvlist */
nv_list * nv_urlstring_to_list (char * str);

/** Lookup the value in an nv_pair which matches grep) */
const char *nv_hash(const nv_list *nvl, const char *grep);

#endif /* __LIB_NV_PAIR_H */


