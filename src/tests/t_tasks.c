/* Copyright (C) 2004-2005 Petta Technology, Inc.
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */

/*
gcc -Wall -DDEBUG -O3 -I. tests/t_tasks.c lib/task.c p_time.c
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>
#include "lib/task.h"

void print_tasks();

int main(void)
{
	task_t task = { VALID_TASK_MAGIC,
			TODO, 
			ICMP, 
			4839903, 
			COND_SUCCESS,
			1, 
			0,
			"4.131.153.3", 
			900913,
			1337,
			42,
			{ 0 },
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0}
			};
	int i;

	task.ip_addr.s_addr = inet_addr("4.131.153.3");

	printf("Size of a single task:  %d bytes.\n", sizeof(task_t));

	printf("Initializing task stack...\n");
	tasks_init();
	
	printf("Now let's add a task.\n");
	i = task_add(task);
	printf("task_add returned %d.\n", i);

	printf("Let's print the tasks:\n");
	print_tasks();

	printf("Get a task to perform...\n");
	i = get_task(&task);
	printf("get_task returned %d.\n", i);

	printf("Did get_task change the status properly?\n");
	print_tasks();

	printf("Pretend to be working for 1 second...\n");
	sleep(1);

	printf("Now let's tell it that we're done with the task.\n");
	i = task_set_status(task.taskid, COMPLETE);
	printf("task_set status returned %d.\n", i);
	print_tasks();

	printf("get_completed\n");
	i = get_completed(&task);
	printf("%d\n", i);
	print_tasks();

	return 0;

}
	
	
	
