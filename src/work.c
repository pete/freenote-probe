/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
	work.c
	Does work.
*/
#include "work.h"

#include "p_time.h"
#include "lib/task.h"
#include "lib/probecast.h"
#include "net-modules/http.h"
#include "net-modules/http-recurse.h"
#include "net-modules/dns.h"

#ifndef WIN32
#include "net-modules/icmp.h"
#endif /* WIN32 */

#include "url-parse.h"
#include "inaddr_none.h"
#include "io.h"
#include "options.h"
#include "err.h"

/*  The task handler array.  handle_task[TYPE](task) handles a given task.  */
static int (*handle_task[MAX_TASK_TYPES])(task_t task);

/*  Master task handler  */
static int do_task(task_t task);

/*  Specific task handlers.  */
static int do_http_task(task_t task);
static int do_dns_task(task_t task);
static int do_icmp_task(task_t task);
static int error_task(task_t task);

/**
	process_tasks 
	Processes tasks.  All the tasks.  All the time.
	Returns 0 on success, -1 on (the theoretical) unrecoverable failure.
*/
int process_tasks()
{
	task_t task;
	int	tmp;
	char	buf[1024];

	probecasts("Status:  Working.\n");

	while(get_task(&task) != -1) {
		tmp = do_task(task);
		if(tmp == -1) {
			io_warning("Problems performing task #%d.\n"
				   "Attempting to continue anyway.\n", 
				   task.taskid);
			task_set_status(task.taskid, IMPOSSIBLE);
		} 

		get_task_id(&task, task.taskid);
		probecasts("Completed:  ");
		task_to_s(buf, task);
		strcat(buf, "\n");
		probecasts(buf);
	}

	return 0;
}

/**
	populate_handlers
	Initializes the array of handlers with the proper functions.
	Call this before attempting to process tasks, or you'll be sorry.
*/
void populate_handlers()
{
	handle_task[INVALID] = error_task;
	handle_task[HTTP_1_0] = do_http_task;
	handle_task[HTTP_1_1] = do_http_task;
	handle_task[HTTPS_1_0] = do_http_task;
	handle_task[HTTPS_1_1] = do_http_task;
	handle_task[DNS] = do_dns_task;
#ifndef CYGWIN32
	handle_task[ICMP] = do_icmp_task;
#else
	handle_task[ICMP] = error_task;
#endif
}

static int do_task(task_t task)
{
	if(!valid_task(task))
		return -1;
	return handle_task[task.type](task);
}

static int do_http_task(task_t task)
{
	http_request *req = url2http(task.data);
	int r;

	if(req == NULL) {
		print_loc();  io_debug("url2http failure (%s)!\n", task.data);
		task.err = COND_OTHER_PROBLEM;
		update_task(task);
		return -1;
	}

	task.ip_addr = req->addr.sin_addr = 
		resolve_cached(req->hostname, NULL, &task.resolve_time);
	if(task.ip_addr.s_addr == INADDR_NONE) {
		print_loc();  io_debug("dns failure (%s)!\n", task.data);
		task.err = COND_DNS_FAILURE;
		http_req_destroy(req);
		update_task(task);
		return -1;
	}
	r = http_get(req);
	if(r == -1) {
		print_loc();  io_debug("http_get failure (%s)!\n", task.data);
		task.err = COND_OTHER_PROBLEM;	//FIXME: http_request should 
						//have error-reporting.
		if(req->status == 302) {	//We never finished redirecting
			task.err = COND_REDIRECTS_EXCEEDED;
		}
	} else
		task.err = COND_SUCCESS;
	task.http_code = req->status;
	task.body_size = req->head_len + req->body_len;

	task.redirect_time = req->redirect_tv;
	task.connect_time = req->connect_tv;
	task.first_byte_time = req->first_byte_tv;
	task.page_time = req->page_tv;
	
	task.ctime = current_time();
	task.components = http_recurse(req, task.data, 
		read_int_opt("max-page-recursion")) - 1;
	task.ctime = time_difference(current_time(), task.ctime);

	http_req_destroy(req);

	update_task(task);
	task_set_status(task.taskid, COMPLETE);
	
	return r;
}

static int do_dns_task(task_t task)
{
	resolve(task.data, NULL, NULL);
	if(task.ip_addr.s_addr == INADDR_NONE) {
		task.err = COND_DNS_FAILURE;
		return 0;
	}
	task_set_status(task.taskid, COMPLETE);
	return 0;
}

static int do_icmp_task(task_t task)
{
	/* TODO:  	The packet size and number of packets are hard-coded
			for now.  In the future, we should probably make this
			configurable by the dispatch server.  I'd rather make
			a way to associate this with the job, though, than to
			just sloppily stuff it into the options.
	*/
#ifndef CYGWIN32
	return icmp_ping(task.data, 4, 56, NULL);
#else
	io_debug("This shouldn't be getting called!\n");
	abort();
#endif /* !CYGWIN32 */
}

static int error_task(task_t task)
{
	print_loc();
	io_err("Attempted to perform invalid task!  taskID was %d.\n", 
		task.taskid);
	task_set_status(task.taskid, IMPOSSIBLE);
	return -1;
}
