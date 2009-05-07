/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "lib/nv_pair.h"
#include "urldecode.h"
#include "util.h"
#include "io.h"

static nv_pair ** nv_pair_init (unsigned size)
{
	int i;
	nv_pair ** nvp = NULL;
	safe_calloc(nvp, ((size + 1) * (sizeof(nv_pair *))));
	for (i = 0; i < size; ++i) {
		nvp[i] = NULL;
	}
	return nvp;
}

static void nv_pair_destroy (nv_pair ** nvp, unsigned size)
{
	int i;
	for (i = 0; i < size; ++i) {
		if (nvp[i]) {
			safe_free(nvp[i]->name);
			safe_free(nvp[i]->value);
			safe_free(nvp[i]);
		}
	}
	safe_free(nvp);
}

unsigned nv_list_insert (nv_list * nvl, const char * name, const char * value,
		unsigned i)
{
	assert(name != NULL);
	assert(value != NULL);

	if (!i && (nvl->last != 0))
		i = nvl->last + 1;

	while(nvl->nvp[i] != NULL)
		++i;

	safe_calloc(nvl->nvp[i],sizeof(nv_pair));
	nvl->nvp[i]->name = strdup(name);
	nvl->nvp[i]->value = strdup(value);
	nvl->data_size += strlen(name);
	nvl->data_size += strlen(value);
	nvl->last = i;

	return i;
}

void nv_list_destroy (nv_list * nvl)
{
	nv_pair_destroy(nvl->nvp, nvl->size);
	safe_free(nvl);
}

nv_list * nv_list_init (unsigned size)
{
	nv_list * nvl = NULL;
	safe_calloc(nvl,sizeof(nv_list));
	nvl->nvp = nv_pair_init(size);
	nvl->size = size;
	nvl->data_size = 0;
	nvl->last = 0;
	return nvl;
}

/**	
	Turns a name/value-pair list (*nvl) into a string.
	Always returns a valid string.  (We abort if we can't allocate any
	memory.)
*/
char * nv_list_to_string (const nv_list *nvl)
{
	char *ret = NULL;
	unsigned seen = 0, i;
	/* create the HTTP body from a nvlist*/
	safe_calloc(ret, (nvl->data_size + (nvl->size * 3) ));
	for (i = 0; i <= nvl->last; ++i) {
		if (nvl->nvp[i] != NULL) {
			if (seen)
				strcat(ret,"&");
			++seen;
			strcat(ret,nvl->nvp[i]->name);
			strcat(ret,"=");
			strcat(ret,nvl->nvp[i]->value);
		}
	}
	return ret;
}

/** nv_urlstring_to_list - converts a urlencodeds string into a new nv_list
 * not using strtok since it can't gracefully handle empty values */
nv_list * nv_urlstring_to_list (char * str)
{
fflush(NULL);
	char *nvp = NULL, *n = NULL, *v, *ev, *stmp, *tmp;
	unsigned count = 1, len, nvplen;
	nv_list * nvl = NULL;
	
	assert(str != NULL);
	
	if(str == NULL)
		return NULL;
	
	len = strlen(str) + 1;
	tmp = str;
	stmp = str;

	while (*(tmp++) != '\0') 
		if (*tmp == '&') 
			++count;

	nvl = nv_list_init(count);
	
	safe_calloc(nvp,len);
	safe_calloc(n,len);
	
	while (*(stmp) != '\0') {
		memset(n,0,len);
		memset(nvp,0,len);
		tmp = strchr(stmp,'&');

		if (tmp == NULL) {
			tmp = stmp;
			while (*(++tmp) != '\0');
		}

		nvplen = strlen(stmp) - strlen(tmp);
		strncpy(nvp, stmp, nvplen);
		v = strchr(nvp, '=');
		++v;
		
		strncat(n, stmp, nvplen - strlen(v) - 1);//SEGFAULT HERE

		ev = urldecode_dup(v);		

		nv_list_insert(nvl,n,ev,0);
		safe_free(ev);
		
		if (*tmp == '\0')
			break;
		stmp = tmp + 1;
	}

	safe_free(nvp);
	safe_free(n);

	return nvl;
}

void nv_list_dump (nv_list * nvl)
{
#ifdef DEBUG
	unsigned int i;
	if (!nvl->nvp)
		return;
	for (i = 0; i < nvl->size; ++i)
		if (nvl->nvp[i] != NULL) {
			io_debug("%i] name=[%s]  value=[%s]\n",
					i, nvl->nvp[i]->name,
					(char *)nvl->nvp[i]->value);
		}
#endif
}

const char *nv_hash(const nv_list *nvl, const char *grep)
{
	int i;
	for(i = 0; i < nvl->size; i++) {
		if (nvl->nvp[i] != NULL) {
			if(!strcmp(grep, nvl->nvp[i]->name))
				return nvl->nvp[i]->value;
		}
	}
	return NULL;
}


