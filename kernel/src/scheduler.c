// scheduler.c - Kernel scheduler
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

//@DISABLE_SWAPS
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include "scheduler.h"
#include "kernel.h"

extern Node tasks[MAX_TASKS];


/**
 * initScheduler - do anything the scheduler needs to before kernel starts
 */
void initScheduler() {
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        tasks[i].tid = NO_TASK;
    }
}

/**
 * scheduler(): return the next task to execute
 * @return: the task id of the next task to execute
 *
 * The scheduler should be a round-robin, preemptive, prioritized
 * scheduler. It should return the next highest priority ready task.
 * The selected task is removed from the scheduler until it is rescheduled.
 */
int scheduler() {
}

/**
 * reschedule: reschedule a task
 * @task: the task to reschedule
 *
 * This function causes a task to be rescheduled.
 */
void reschedule(int task) {
    return;
}

/**
 * deschedule: remove a task from the ready queue
 * @task: the task to remove
 *
 * This function causes a task to be removed from the ready queue.
 */
void deschedule(int task) {
}

void clearScheduler() {
}

//@ENABLE_SWAPS
