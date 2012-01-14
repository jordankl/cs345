#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "semaphores.h"
#include "scheduler.h"
#include "delta_clock.h"
#include "system_calls.h"
#include "signals.h"
#include "kernel.h"
#include "type_defs.h"

/**
 * myint - send KSIGINT after n seconds
 */
int myint(int argc, char* argv[])
{
	tid_t tid = gettid();
	char name[128];
    int secs;
	int s = NULL_SEMAPHORE;
	int i;

    if (argc != 2) {
		fprintf(stderr, "Usage: %s <n>\n", argv[0]);
		return 1;
    }
    secs = atoi(argv[1]);
	sprintf(name, "P1_myint - delta clock %d secs - tid %d", secs, tid);
	s = createSemaphore(name, BINARY, 0);
	assert(s != NULL_SEMAPHORE);
	insertDeltaClock(10,s,1);

	// Wait until you are ticked
	for (i = 0; i < secs; i++) {
		semWait(s);
	}

	deleteClockEvent(s);
	deleteSemaphore(s);

    if (sigKill(tid, KSIGINT) < 0) {
		fprintf(stderr, "sigKill (int) error");
	}

	// Should never reach here because it gets killed first
	printf("Shouldn't reach here in myint\n");
	return 0;
}

/**
 * myspin - run for n seconds, then return
 */
int myspin(int argc, char* argv[]) {
    int secs;
	tid_t tid = gettid();
	char name[128];
	int s = NULL_SEMAPHORE;
	int i;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <n>\n", argv[0]);
		return 3;
    }

    secs = atoi(argv[1]);

	sprintf(name, "P1_myspin - delta clock %d secs - tid %d", secs, tid);
	s = createSemaphore(name, BINARY, 0);
	assert(s);
	insertDeltaClock(10,s,1);

	// Wait until you are ticked
	for (i = 0; i < secs; i++) {
		semWait(s);
	}

	deleteClockEvent(s);
	deleteSemaphore(s);

    return 0;
}

/**
 * mysplit - start a task, wait, then return
 */
int mysplit(int argc, char* argv[])
{
	tid_t tid;

    if (argc != 2) {
		fprintf(stderr, "Usage: %s <n>\n", argv[0]);
		return -1;
    }

	NewTask mySplitTask;
	mySplitTask.name = "myplit";
	mySplitTask.task = myspin;
	mySplitTask.priority = MED_PRIORITY;
	mySplitTask.argc = argc;
	mySplitTask.argv = argv;
	mySplitTask.parentHandlers = TRUE;
	mySplitTask.tgidNew = FALSE;
	tid = createTask(&mySplitTask);

    /* parent waits for child to terminate */
    wait_b(NULL);

	return 0;
}

/**
 * mystop - run for n seconds, then send KSIGSTOP to itself
 */
int mystop(int argc, char* argv[]) {
	tid_t tid = gettid();
	char name[128];
    int secs;
	int s = NULL_SEMAPHORE;
	int i;

    if (argc != 2) {
		fprintf(stderr, "Usage: %s <n>\n", argv[0]);
		return 1;
    }

    secs = atoi(argv[1]);

	sprintf(name, "P1_mystop - delta clock %d secs - tid %d", secs, tid);
	s = createSemaphore(name, BINARY, 0);
	assert(s);
	insertDeltaClock(10,s,1);

	// Wait until you are ticked
	for (i = 0; i < secs; i++) {
		semWait(s);
	}

	deleteClockEvent(s);
	deleteSemaphore(s);

    if (sigKill(tid, KSIGTSTP) < 0) {
		fprintf(stderr, "sigKill (int) error");
	}

	return 0;
}


int testScheduler(int argc, char* argv[]) {
    return 0;
}
