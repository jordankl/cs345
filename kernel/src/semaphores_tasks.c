// signals_tasks.c - Multi-tasking
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "semaphores_tasks.h"
#include "system_calls.h"
#include "kernel.h"

#define COUNT_MAX	5

int s1Sem;					// task 1 semaphore
int s2Sem;					// task 2 semaphore

extern Semaphore* semaphoreList;			// linked list of active semaphores
extern jmp_buf reset_context;				// context of kernel stack

int testSignals(int argc, char* argv[])
{
	static char* s1Argv[] = {"signal1", "s1Sem"};
	static char* s2Argv[] = {"signal2", "s2Sem"};
	static char* aliveArgv[] = {"ImAliveTask", "3"};

	// start tasks looking for sTask semaphores
	NewTask signal1TaskInfo;
	signal1TaskInfo.name = "signal1";
	signal1TaskInfo.task = signalTask;
	signal1TaskInfo.priority = VERY_HIGH_PRIORITY;
	signal1TaskInfo.argc = 2;
	signal1TaskInfo.argv = s1Argv;
	signal1TaskInfo.parentHandlers = FALSE;
	signal1TaskInfo.tgidNew = FALSE;
	createTask(&signal1TaskInfo);

	NewTask signal2TaskInfo;
	signal2TaskInfo.name = "signal2";
	signal2TaskInfo.task = signalTask;
	signal2TaskInfo.priority = VERY_HIGH_PRIORITY;
	signal2TaskInfo.argc = 2;
	signal2TaskInfo.argv = s2Argv;
	signal2TaskInfo.parentHandlers = FALSE;
	signal2TaskInfo.tgidNew = FALSE;
	createTask(&signal2TaskInfo);

	NewTask ImAliveTask1Info;
	ImAliveTask1Info.name = "ImAliveTask1";
	ImAliveTask1Info.task = ImAliveTask;
	ImAliveTask1Info.priority = LOW_PRIORITY;
	ImAliveTask1Info.argc = 2;
	ImAliveTask1Info.argv = aliveArgv;
	ImAliveTask1Info.parentHandlers = FALSE;
	ImAliveTask1Info.tgidNew = FALSE;
	createTask(&ImAliveTask1Info);

	NewTask ImAliveTask2Info;
	ImAliveTask2Info.name = "ImAliveTask2";
	ImAliveTask2Info.task = ImAliveTask;
	ImAliveTask2Info.priority = LOW_PRIORITY;
	ImAliveTask2Info.argc = 2;
	ImAliveTask2Info.argv = aliveArgv;
	ImAliveTask2Info.parentHandlers = FALSE;
	ImAliveTask2Info.tgidNew = FALSE;
	createTask(&ImAliveTask2Info);

	return 0;
}

/**
 * signal1
 */
int signal1(int argc, char* argv[])		// signal1
{
	semSignal(s1Sem);
	return 0;
}

/**
 * signal2
 */
int signal2(int argc, char* argv[])		// signal2
{
	semSignal(s2Sem);
	return 0;
}

/**
 * signalTask
 */
int signalTask(int argc, char* argv[])
{
	int count = 0;					/* task variable */
	TCB* tcb = getTCB();
	int curTask = gettid();

	// create a semaphore
	int* mySem = (!strcmp(argv[1], "s1Sem")) ? &s1Sem : &s2Sem;
	*mySem = createSemaphore(argv[1], 0, 1);

	// loop waiting for semaphore to be signaled
	while(count < COUNT_MAX)
	{
		semWait(*mySem);
		printf("%s  Task[%d], count=%d\n", tcb[curTask].name, curTask, ++count);
	}

	return 0;
}

/**
 * ImAliveTask
 */
int ImAliveTask(int argc, char* argv[])
{
	int i;
	int curTask = gettid();

	while (1)
	{
		printf("\n\t(Task %d) I'm Alive!\n", curTask);
		for (i = 0; i < 100000; i++) {
			swapTask();
		}
	}
	return 0;
}

