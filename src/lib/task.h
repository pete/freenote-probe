/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#ifndef __LIB_TASK_H
#define __LIB_TASK_H

#include <time.h>
#include <sys/time.h>

#ifdef WIN32
# define __INSIDE_CYGWIN__
# include <winsock.h>
#else
# include <netinet/in.h>
#endif /* WIN32 */

#define VALID_TASK_MAGIC 0x9E3779B9	/* (1 + sqrt(5))/2 ~= 0x1.9E3779B9 */
#define MAX_TASKS 15	/* Right now, the dispatch server only gives 10. */


typedef enum { 
	INVALID, 	/* Null tasks have no status. */
	TODO, 
	RUNNING, 
	COMPLETE,
	REPORTED,
	IMPOSSIBLE,	/* For impossible or badly formed tasks. */
} task_stat;

typedef enum { 
	NULL_TASK, 	/* Meaning that this slot in the array is empty */
	HTTP_1_0, 
	HTTP_1_1, 
	HTTPS_1_0, 
	HTTPS_1_1, 
	DNS, 
	ICMP,
	MAX_TASK_TYPES	/* Must be the last task, and never the type of a task.
			   It's a constant for the maximum number of different
			   task types we can have.  Putting it here keeps us
			   from having to change it, and mostly eliminates 
			   human error.  */
} task_type;

typedef enum {
	COND_UNDEFINED = 0,
	COND_SUCCESS = 1,
	COND_DNS_FAILURE = 2,
	COND_HTTP_TIMEOUT = 3,
	COND_REDIRECTS_EXCEEDED = 4,
	COND_OTHER_PROBLEM = 999,
	/* Fill in more later */
} error_cond;

typedef struct {
	int magic;
	task_stat status;	/* Current status */
	task_type type;		/* Task type      */
	error_cond err;
	
	/* 	This task's unique ID.  A negative ID means it is a sub-task
		created by the probe, and a positive ID is one that was 
		assigned by the dispatch server.
	*/
	int taskid;
	int workid;
	int parent;	/* 0 for tasks direct from the dispatch server */

	/*	This is for a URL to fetch, a hostname to look up, a server to 
		ping, things like that.
	*/
	char data[1024];


	/* Statistics we need to report back.  */
	int body_size;			/* $SIZE_BYTES */
	int http_code;			/* $HTTP_RESULT */
	int components;			/* $TOTAL_COMPONENTS */
	struct in_addr ip_addr;		/* $IP_RESOLVED */
	struct timeval resolve_time;	/* $DNS_MS */
	struct timeval redirect_time;	/* $REDIRECT_MS */
	struct timeval connect_time;	/* $CONNECT_MS */
	struct timeval first_byte_time;	/* $FIRST_BYTE_MS */
	struct timeval ctime;		/* $COMPONENT_MS */
	struct timeval page_time;	/* $PAGE_MS */
	struct timeval total_time;	/* $TOTAL_MS */
	struct timeval time_started;	
} task_t;

void tasks_init();
void tasks_clear();
int task_add(task_t t);
task_t nv_to_task(int workid, char *urlid, char *desc);
int get_task(task_t *t);
int get_completed(task_t *t);
int get_task_id(task_t *t, int id);
int task_set_status(int taskid, task_stat status);
int valid_task(const task_t task);
int update_task(task_t task);
int task_to_s(char *buf, task_t t);
void print_tasks();

#endif /* __LIB_TASK_H */

