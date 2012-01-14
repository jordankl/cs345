#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "delta_clock_tasks.h"
#include "type_defs.h"
#include "kernel.h"
#include "system_calls.h"

/**
 *
 */
int testDeltaClock(int argc, char* argv[])
{
	// Implement a routine to test the delta clock
	printf("Testing Delta Clock\n");
	
	static char* deltaClockTestTaskArgv[] = {"deltaClockTestTask"};
	
	// Store Random Number generator
	srand((unsigned int)time(NULL));
	
	// Create 10 Delta Clock Test Tasks
	int i;
	for (i = 0; i < 20; i++)
	{
		NewTask deltaClockTest;
		deltaClockTest.name = "DeltaClockTest";
		deltaClockTest.task = deltaClockTestTask;
		deltaClockTest.priority = MED_PRIORITY;
		deltaClockTest.argc = 1;
		deltaClockTest.argv = deltaClockTestTaskArgv;
		deltaClockTest.parentHandlers = FALSE;
		deltaClockTest.tgidNew = TRUE;
		createTask(&deltaClockTest);
	}
	
	return 0;
}

/**
 *
 */
int deltaClockTestTask(int argc, char* argv[])
{
	// Generate a random number for time
	int eventTime = (rand() % 50)*5 + 1;
	
	// Get taskId of this task
	int taskId = gettid();
	
	int semId = createSemaphore("deltaClockTestTaskSem", BINARY, 0);
	
	if (KDEBUG) printf("Inserting Task %d into delta clock with time=%d\n",taskId, eventTime);
	
	// Create a Clock Event Blocking on this task's semaphore
	if (insertDeltaClock(eventTime, semId, 0) < 0)
	{
		printf("There was an error trying to insert Semaphore ID %d with time %d\n",semId, eventTime);
	}
	
	// Wait
	semWait(semId);
	
	// Get current time
	time_t currentTime;
	time(&currentTime);
	
	printf("Task %d just finished at %s\n", taskId, ctime(&currentTime));
	return 0;
}

/**
 *
 */
int listDeltaClockTask(int argc, char* argv[])
{
	// Do System Call
	listDeltaClock();
	
	// Do System Call
	return 0;
}
