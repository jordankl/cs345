#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "delta_clock.h"
#include "system_calls.h"
#include "kernel.h"


/**
 * struct clockEvent - an event structure for the delta clock
 * @init_value: the initial value of the delay
 * @time: the current number of ticks until signaled
 * @sem: the semaphore that will be signaled upon completion
 * @periodic: this value determines if the event should be repeated
 * @next: the pointer to the next event in the linked list
 */
typedef struct event
{
	int init_value;
	int time;
	int semId;
	int periodic;
	struct event* next;
} clockEvent;

// Delta Clock Variables
clockEvent* deltaClock = 0;
extern int taskSemIds[MAX_TASKS];			// task semaphore
extern int superMode;						// system mode

//@DISABLE_SWAPS



unsigned long myClkTime;
unsigned long myOldClkTime;


/**
 * _insertDeltaClock: insert a semaphore into the delta clock
 * @time: the delay in tenths of a second
 * @sem: the semaphore to be signaled at the end of the delay
 * @periodic: if the delay/signal should be repeated
 * @return: 0 if successful, -1 if failed
 *
 * This function should insert a clock event into the delta clock
 * in the appropriate place.  If the time is less than 0 or if the
 * semaphore is invalid, the function should return -1.  If this time
 * is 0, the semaphore should be signaled before returning.
 */
int _insertDeltaClock(int time, int semId, int periodic)
{
    assert("Error: Trying to call _insertDeltaClock in User Mode\n" && superMode);

	return -1;
}

/**
 * _deleteClockEvent: remove clock events from the delta clock
 * @sem: the semaphore whose clock events should be deleted
 * @return: 1 if successful, 0 if failed
 *
 * This function should remove all clock events from the delta clock
 * associated with the sempahore identified with semId. The delta
 * clock should be traversed to find all clock events associated
 * with the matching semaphore.  The events should be removed from the
 * linked list properly and any allocated memory should be freed.
 */
int _deleteClockEvent(int semId)
{
	return 0;
}

/**
 * _tickDeltaClock: tick delta clock and signal any finished events
 *
 * Tick the delta clock on a 1/10th second resolution signaling events
 * as appropriate.  The clock should maintain a notion of time of last
 * call to manage clock drift as it is only called in pollInterrupts.
 * As semaphores are signaled, if the event is periodic, it should be
 * added back to the delta clock.
 */
int _tickDeltaClock() {
    assert("Error: Trying to call _tickDeltaClock in User Mode\n" && superMode);
 	return 0;
 }


/**
 *
 */
int _listDeltaClock()
{
    assert("Error: Trying to call _listDeltaClock in User Mode\n" && superMode);
    puts("*********************");
    puts("Delta Clock Contents");
    // delta clock queue is empty
    if (!deltaClock) {
        puts("Empty");
    }
    // Print all of the delta clock queue's contents
    else {
        clockEvent * n = deltaClock;
        while (n) {
            printf("Semaphore %s with time=%d\n", _getSemaphore(n->semId)->name, n->time);
            n = n->next;
        }
    }
    puts("*********************");
	return 0;
}

//@ENABLE_SWAPS
