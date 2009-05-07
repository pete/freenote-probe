/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <stdio.h>
#include <assert.h>

#include "inaddr_none.h"
#include "util.h"
#include "lib/libares/ares.h"
#include "dns.h"
#include "p_time.h"
#include "io.h"
#include "err.h"

static dns_centry dns_cache[MAX_DNS_CACHE_SIZE];

static void callback(void *arg, int status, struct hostent *host);
static void cache_each_do(void (*run)(dns_centry *));
static int cache_itest(int (*test)(dns_centry, const void *), 
	const void *data);
static int is_cached(const char *name);
static int add_cache(const char *name, struct in_addr addr);
static void cacheinit(dns_centry *i);
static void freecache(dns_centry *i);


/*
	dns_init
	Initializes the DNS cache.  Call this before using resolve_cached().
*/
void dns_init()
{
	cache_each_do(cacheinit);
}

/*
	dns_cache_flush
	Flushes the DNS cache.  
*/
void dns_cache_flush()
{
	cache_each_do(freecache);
}

/*
	resolve_cached
	Attempts to resolve a DNS entry by looking through the cache.  If 
	the name is not cached, it is resolved and placed in the cache.  
	Arguments and return value are the same as for resolve().
*/
struct in_addr resolve_cached(const char *name, const char *ns, 
	struct timeval *time)
{
	struct in_addr addr;
	int n;
	
	if(time != NULL)
		*time = current_time();
	n = is_cached(name);
	if(n == -1) {
		addr = resolve(name, ns, time);
		add_cache(name, addr);
		return addr;
	}
	if(time != NULL)
		*time = time_difference(current_time(), *time);
	return dns_cache[n].addr;
}

/*
	resolve
	Given a hostname to resolve (name), an optional nameserver to use
	(nameserver_ip), and an (also optional) timeval struct to store the
	time it takes to perform the job (time), it will return an in_addr 
	struct that contains either the resolved address or INADDR_NONE in
	case of an error.
*/
struct in_addr resolve(const char *name, const char *nameserver_ip, 
	struct timeval *time)
{
	struct in_addr addr;
#ifndef CYGWIN32
	ares_channel channel;
	int status, nfds, ip_i;
	fd_set read_fds, write_fds;
	struct timeval *tvp, tv;
	char *errmem;
	char ip[16] = "---------------";

	status = ares_init(&channel);
	if (status != ARES_SUCCESS) {
		print_loc();
		io_err("ares_init: %s\n",
			ares_strerror(status, &errmem));
		ares_free_errmem(errmem);
		addr.s_addr = INADDR_NONE;
		return addr;
	}

	print_loc();
	io_debug("Resolving %s using", name);
	if (nameserver_ip != NULL) {
		ip_i = inet_addr(nameserver_ip);
		if(ip_i != -1)
			channel->servers->addr.s_addr = ip_i;
	}
	io_debug(" %s\n", inet_ntoa(channel->servers->addr));

	if(time != NULL)
		*time = current_time();
	ares_gethostbyname(channel, name, AF_INET, callback, ip);

	/* Wait for all queries to complete. */
	while (ip[0] == '-') {
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		nfds = ares_fds(channel, &read_fds, &write_fds);
		if (!nfds)
			break;
		tvp = ares_timeout(channel, NULL, &tv);
		select(nfds, &read_fds, &write_fds, NULL, tvp);
		ares_process(channel, &read_fds, &write_fds);
	}

	if(time != NULL)
		*time = time_difference(*time, current_time());

	ares_destroy(channel);
	addr.s_addr = inet_addr(ip);

#else	/* CYGWIN32 is defined */
	struct hostent *host;
	if(nameserver_ip != NULL) {
		print_loc();
		io_debug("Cannot specify nameserver in this OS!!!\n");
	}
	if(time != NULL)
		*time = current_time();
	host = gethostbyname(name);
	
	if(host != NULL)
        	memcpy(&addr, *(host->h_addr_list), sizeof(struct in_addr));
	else
		addr.s_addr = INADDR_NONE;
	if(time != NULL)
		*time = time_difference(*time, current_time());
#endif  /* CYGWIN32 */
	return addr;
}

static void callback(void *arg, int status, struct hostent *host)
{
	struct in_addr addr;
	char *mem;


	if (status != ARES_SUCCESS) {
		print_loc();
		io_debug("%s: %s\n", (char *)arg, ares_strerror(status, &mem));
		ares_free_errmem(mem);
		strcpy((char *)arg, "255.255.255.255");
		return;
	}

	/* We only care about one address for now. */
	memcpy(&addr, *(host->h_addr_list), sizeof(struct in_addr));
	strcpy((char *)arg, inet_ntoa(addr));
}

/*
	cache_each_do
	Runs the given function on each member of the cache.
*/
static void cache_each_do(void (*run)(dns_centry *))
{
        int i;
        for(i = 0; i < MAX_DNS_CACHE_SIZE; i++) {
                run(&dns_cache[i]);
        }
}

/*
	cache_itest
	Provided a test function and data to test against, it returns an index
	if the test returns nonzero for any member of the cache.
*/
static int cache_itest(int (*test)(dns_centry, const void *), const void *data)
{
	int i;
	for(i = 0; i < MAX_DNS_CACHE_SIZE; i++) {
		if(dns_cache[i].name != NULL && test(dns_cache[i], data))
			return i;
	}
	return -1;
}

static int same_name(dns_centry i, const void *fname)
{
	if(!strcmp(fname, i.name))
		return 1;
	return 0;
}

/*
	is_cached
	Given a name, returns an index if the value is cached or -1 if it
	is not.
*/
static int is_cached(const char *name)
{
	return cache_itest(same_name, name);
}

/*
	add_cache
	Adds a dns_centry to the cache.
*/
static int add_cache(const char *name, struct in_addr addr)
{
	static unsigned char index = 0;
	unsigned char i;
	assert(name != NULL);
	
	i = (index++ % MAX_DNS_CACHE_SIZE);

	freecache(&dns_cache[i]);
	dns_cache[i].name = strdup(name);

	if(dns_cache[i].name == NULL)
		return -1;
	
	dns_cache[i].addr = addr;

	return 0;
}

static void cacheinit(dns_centry *i)
{
	i->name = NULL;
	i->addr.s_addr = INADDR_NONE;
}

static void freecache(dns_centry *i)
{
	safe_free(i->name);
	i->addr.s_addr = INADDR_NONE;
}


