/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>	/* inet_ntoa */
#include <netinet/in.h>	/* INADDR_NONE */

#include "inaddr_none.h"
#include "lib/task.h"
#include "lib/proto.h"
#include "p_time.h"
#include "io.h"
#include "err.h"

/*
	This contains our tasks.  Not to be touched from the outside.
*/
static task_t tasks[MAX_TASKS];

/*  Search functions  */
static int find_with_status(task_stat s);
static int find_with_type(task_type t);
static int find_with_id(int id);
/* static int find_with_parent(int parent); */

/*  Task management  */
static int tasks_itest(int (*test) (task_t *, int), int test_data);
static void tasks_each_do(void (*run) (task_t *));
static void tasks_test_do(int (*test) (task_t *, int), int test_data,
			  void (*run) (task_t *));
static void init_task(task_t * t);
static void task_copy(task_t * dest, const task_t * src);
static void mark_impossible(int parentid);

/*  Test functions.  */
/*  I've been keeping these nested to prevent clutter (it's also easier to
 *  read when the function is right there), but I've pulled out the ones
 *  that are shared so that I don't have to re-define them.
 */
static int paternity_test(task_t * t, int parent);

#define NEXTINLIST(p, r)	do {					\
			(p) = strchr((p) + 1, ',');			\
			if((p) == NULL)					\
				return (r);				\
			p++;						\
			} while (0)

/**
	tasks_init
	Call this before attempting to use the task stack.
*/
void tasks_init()
{
	tasks_each_do(init_task);
}

/**
	tasks_clear
	Call this to reset the task stack.
*/
void tasks_clear()
{
	/* Because no memory is dynamic for now, we don't need to free()
	   any memory or anything like that.  Essentially, it's the same
	   as just calling tasks_init().
	*/
	tasks_init();
}

/**
	task_add 
	Add a task.  Returns 0 on success, -1 on failure.  Possible reasons for
	failure are that task_add was passed an invalid task, or could not find 
	an empty slot.
*/
int task_add(task_t t)
{
	int i = find_with_type(NULL_TASK);

	if(t.magic != VALID_TASK_MAGIC)	/* Bad magic. */
		return -1;

	if (i == -1)		/*  No empty tasks!  */
		return -1;

	task_copy(&tasks[i], &t);
	tasks[i].status = TODO;

	return 0;
}

/**
	nv_to_task
	Takes a name and a value, and converts them into a task.  Returns
	a valid task if it could parse the data, an invalid task otherwise.
	Designed to work like this:
		t = nv_to_task(taskid, taskdata);
		if(valid_task(t))
			task_add(t);
		else
			// Error condition...

	desc should be of the form "TYPE,ACTION,URL"
*/
task_t nv_to_task(int workid, char *urlid, char *desc)
{
	task_t r;
	char *p = desc;

	init_task(&r);
	r.workid = workid;
	r.taskid = atoi(urlid);
	if(!r.taskid)
		return r;
	r.type = atoi(p);	/* R-Type!! */
	NEXTINLIST(p, r);
	NEXTINLIST(p, r);
	/* FIXME:  We ignore 'action' for now. */

	/* If we made it here, we're successful. */
	strcpy(r.data, p);
	r.magic = VALID_TASK_MAGIC;
	r.status = TODO;
	return r;
}

/**
	task_to_s
	Given a pointer to a character buffer and a task id, task_to_s
	converts it to a string that can be sent to the collection server and 
	puts it in the buffer.  Please ensure that there is enough space in the
	buffer.
	Returns 0 for failure, the size of the string otherwise. 
*/
int task_to_s(char *buf, task_t t)
{
	if(t.magic != VALID_TASK_MAGIC)
		return 0;
	/* Now it gets ugly. */
	return sprintf(buf, "%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		t.type,
		t.err,
		t.http_code,
		inet_ntoa(t.ip_addr),
		t.body_size,
		time_ms(t.resolve_time),
		time_ms(t.redirect_time),
		time_ms(t.connect_time),
		time_ms(t.first_byte_time),
		time_ms(t.page_time),
		time_ms(t.ctime),
		t.components,
		time_ms(t.total_time));
}

/**
	get_task
	Gives you a task to perform.  Starts the timer ticking.
	Returns -1 if there are no tasks to perform, 0 otherwise.
*/
int get_task(task_t * t)
{
	int i = find_with_status(TODO);

	if (i == -1)		/* No tasks to do! */
		return -1;

	tasks[i].status = RUNNING;
	tasks[i].time_started = current_time();	/* The clock is ticking... */

	task_copy(t, &tasks[i]);

	return 0;
}

/**
	get_completed
	Gives you a task that has been completed, and removes that task from
	the stack.  You should call this to send data to the collection
	server.  The return value is 0.

	If there are no tasks that have been completed, but there were tasks
	that we could not complete, then one of those is returned instead.  The
	return value is 1.

	Otherwise, no task is sent back and the return value is -1.
*/
int get_completed(task_t * t)
{
	int rt, r = 0;
	char buf[1024];

	rt = find_with_status(COMPLETE);
	if(rt == -1) {
		rt = find_with_status(IMPOSSIBLE);
		if(rt == -1)
			return -1;
		r = 1;
	}


	task_copy(t, &tasks[rt]);
	init_task(&tasks[rt]);

	return r;
}

/**
	get_task_id
	Like get task, but allows for requesting a specific task.
	Gets a task with the specified ID.  Returns 0 for success, -1 for 
	failure.  Calling it with a null argument is a way to see if a task 
	with that ID exists.  If you provide a non-null place to put a task, 
	it will start the task running and copy the it.
*/
int get_task_id(task_t * t, int id)
{
	int i = find_with_id(id);

	if (i == -1)
		return -1;

	if (t != NULL) {
		if(tasks[i].status == TODO) {
			tasks[i].status = RUNNING;
			tasks[i].time_started = current_time();
		}
		task_copy(t, &tasks[i]);
	}

	return 0;
}

/**
	task_set_status
	Sets the status of the task with the given taskid.  Returns 0 for
	success, -1 if there is no task with that id.
*/
int task_set_status(int taskid, task_stat status)
{
	int i = find_with_id(taskid);

	if (i == -1)
		return -1;

	tasks[i].status = status;

	/* Two special cases. */
	if(status == COMPLETE) {
		tasks[i].total_time = time_difference(tasks[i].time_started,
						current_time());
	} else if(status == IMPOSSIBLE) {
		print_loc();
		io_debug("Impossible task:  %d\n", taskid);
		mark_impossible(taskid);
	}
	return 0;
}

/**
	valid_task
	Examines the validity of a task.  Returns 0 for invalid tasks,
	and 1 for valid tasks.
*/
int valid_task(const task_t task)
{
	if((task.magic == VALID_TASK_MAGIC) &&
	   (task.status != INVALID) &&
	   (task.type != NULL_TASK)) 
		return 1;
	print_loc();
	io_debug("Invalid task:\n\tmagic: %x, status: %d, type:  %d\n", 
			task.magic, task.status, task.type);
	return 0;
}

/**
	update_task
	Updates the task with new data.
	Returns 0 for success and -1 for failure.
*/
int update_task(task_t task)
{
	int i = find_with_id(task.taskid);
	
	if(i == -1)
		return -1;

	task.magic = tasks[i].magic;
	task.status = tasks[i].status;
	task.type = tasks[i].type;
	task.workid = tasks[i].workid;

	task_copy(&tasks[i], &task);
	return 0;
}

/*
	find_with_status
	Looks through the tasks for one with the specified status, and
	returns its index if a task was found, -1 otherwise.
*/
	int statustest(task_t * t, int status) 
	{
		if (t->status != status)
			return -1;
		return 0;
	}
static int find_with_status(task_stat s)
{
	return tasks_itest(&statustest, s);
}

/*
	find_with_type
	Looks through the tasks for one with the specified type, and
	returns its index if a task was found, -1 otherwise.
*/
	int typetest(task_t * t, int type) 
	{
		if (t->type != type)
			return -1;
		return 0;
	}
static int find_with_type(task_type t)
{
	return tasks_itest(&typetest, t);
}

/*
	find_with_id
	Looks through the tasks for one with the specified ID, returning either
	its index on success, or -1 on failure.
*/
	int idtest(task_t * t, int id) 
	{
		if (t->taskid != id)
			return -1;
		return 0;
	}
static int find_with_id(int id)
{
	return tasks_itest(&idtest, id);
}

#if 0	/* Not used right now. */
/*
	find_with_parent
	Looks for a task whose parent's id is equal to the id supplied.
*/
static int find_with_parent(int parent)
{
	return tasks_itest(&paternity_test, parent);
}
#endif /* 0 */

/*
	tasks_itest

	#!/usr/bin/env ruby
	tasks.each { |task| return task if test.call(task) }
	return nil
*/
static int tasks_itest(int (*test) (task_t *, int), int test_data)
{
	int i;
	for (i = 0; i < MAX_TASKS; i++) {
		if (test(&tasks[i], test_data) != -1) {
			return i;
		}
	}
	return -1;
}

/*
	tasks_each_do
	Takes (a function that takes an int as an argument) as an argument.
	Runs the provided function on every task in our stack.
	This prevents us from having to re-write iteration code in every place
	it appears.  If, for example, we change to a linked list or a null-
	terminated array of pointers, we only change the code here, not in 
	all the places we use it.  This makes changes trivial.
*/
static void tasks_each_do(void (*run) (task_t *))
{
	tasks_test_do(NULL, 0, run);
}

static void tasks_test_do(int (*test) (task_t *, int), int test_data,
			  void (*run) (task_t *))
{
	int i;
	if (test != NULL) {
		for (i = 0; i < MAX_TASKS; i++)
			if (test(&tasks[i], test_data) != -1)
				run(&tasks[i]);
	} else {
		for (i = 0; i < MAX_TASKS; i++)
			run(&tasks[i]);
	}
}


/*
	init_task
	Initializes a task.
*/
static void init_task(task_t * t)
{
	const task_t null_task = {
		~VALID_TASK_MAGIC,	/* magic */
		INVALID,	/* status */
		NULL_TASK,	/* type */
		COND_UNDEFINED,	/* err */
		0,		/* urlid */
		0,		/* workid */
		0,		/* parent */
		"\0",		/* data */

		0, 		/* body_size */
		0,		/* http_code */
		0, 		/* components */
		{ INADDR_NONE },/* ip_addr */
		{0, 0},		/* resolve_time */
		{0, 0},		/* redirect_time */
		{0, 0},		/* connect_time */
		{0, 0},		/* first_byte_time */
		{0, 0},		/* ctime */
		{0, 0},		/* page_time */
		{0, 0},		/* total_time */
		{0, 0}		/* time_started */
	};

	task_copy(t, &null_task);
}

/*
	task_copy
	Copies src to dest.
*/
static void task_copy(task_t * dest, const task_t * src)
{
	memcpy(dest, src, sizeof(task_t));
}

/*
	mark_impossible
	Marks all of a task's children impossible.
*/
void mark(task_t *t) 
{
	print_loc();
	io_debug("Task marked impossible:  %d\n", t->taskid);
	t->status = IMPOSSIBLE;	/* This task is impossible! */
	mark_impossible(t->taskid);/* Dependencies are impossible! */
	print_tasks();
}
static void mark_impossible(int parentid)
{
	tasks_test_do(paternity_test, parentid, mark);
}

static int paternity_test(task_t * t, int parent) 
{
	if (t->parent != parent)
		return -1;
	return 0;
}

#ifdef DEBUG
void print_task(task_t * t) 
{
	char buffer[4096];
	int i;
	if (t->type == NULL_TASK)
		return;
	i = task_to_s(buffer, *t);
	printf
	    ("t = 0x%X\n"
	     "---(%d)\n%s\n---\n"
	     "magic:[%X]\n status:[%d]\n type:[%d]\n taskid:[%d]\n workid"
	     ":[%d]\n parent:[%d]\n body_size:[%d]\n ip_addr:[%x]\n http_c"
	     "ode:[%d]\n components:[%d]\n resolve_time:[%d.%06d]\n redi"
	     "rect_time:[%d.%06d]\n connect_time:[%d.%06d]\n first_byt"
	     "e_time:[%d.%06d]\n ctime:[%d.%06d]\n page_time"
	     ":[%d.%06d]\n total_time:[%d.%06d]\n time_started:[%d.%06d]"
	     "\nerror:[%d]"
	     "\n data(0x%X):  [%s]\n", 
	     t,
	     i, buffer,
	     t->magic, t->status, t->type, t->taskid, 
	     t->workid, t->parent, t->body_size, t->ip_addr, 
	     t->http_code, t->components,
	     t->resolve_time.tv_sec, 
	     t->resolve_time.tv_usec, t->redirect_time.tv_sec, 
	     t->redirect_time.tv_usec, t->connect_time.tv_sec, 
	     t->connect_time.tv_usec, t->first_byte_time.tv_sec, 
	     t->first_byte_time.tv_usec, t->ctime.tv_sec,
	     t->ctime.tv_usec, t->page_time.tv_sec,
	     t->page_time.tv_usec, t->total_time.tv_sec,
	     t->total_time.tv_usec, t->time_started.tv_sec,
	     t->time_started.tv_usec, t->err, t->data, t->data);
}
void print_tasks()
{
	tasks_each_do(print_task);
}
#else  // DEBUG defined?
void print_tasks()
{
}
#endif /* DEBUG defined? */

#undef NEXTINLIST


