#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "semaphores.h"
#include "delta_clock.h"
#include "kernel.h"
#include "scheduler.h"

extern int superMode;
Semaphore* semaphoreArray[MAX_SEMAPHORES];
int nextSemaphoreId = 0;
int totalSemaphores = 0;


//@DISABLE_SWAPS

/**
 * _createSemaphore - create a semaphore
 * @name: the name of the semaphore
 * @type: the type of the semaphore (BINARY or COUNTING)
 * @state: the initial state of the semaphore
 * @return: the new id of the semaphore
 *
 * This function should allocated memory for a new semaphore and should
 * initialize all of its values.  It should also check that a semaphore
 * with the same name has not been created already.
 */
int _createSemaphore(char* name, int type, int state)
{
	assert("Error: Trying to call _createSemaphore in User Mode\n" && superMode);

	int curTask = _gettid();
	int semId;
	int i;
	Semaphore* sem;

	// assert semaphore is binary or counting
	assert("createSemaphore Error" && ((type == 0) || (type == 1)));	// assert type is validate

	// Check to see if the semaphore has already been defined
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		sem = semaphoreArray[i];

		// Make sure this is a valid location
		if (sem == NULL) {
			continue;
		}

		if (!strcmp(sem->name, name)) {
			sem->type = type;				// 0=binary, 1=counting
			sem->state = state;				// initial semaphore state
			sem->taskNum = curTask;			// set parent task #
			return i;
		}
	}

	// Get the next Semaphore Id
	semId = _getNextSemaphoreId();

	// Allocate Memory for Semaphore
	sem = (Semaphore*)malloc(sizeof(Semaphore));

	// Initialize Semaphore
	sem->id = semId;
	sem->name = (char*)malloc(strlen(name)+1);
	strcpy(sem->name, name);				// semaphore name
	sem->type = type;						// 0=binary, 1=counting
	sem->state = state;						// initial semaphore state
	sem->taskNum = curTask;					// set parent task #

	// Add the semaphore to the global array
	semaphoreArray[semId % MAX_SEMAPHORES] = sem;

	// NULL out the blocked list
	sem->blockedList = makeQueue(0);

	if (KDEBUG) printf("createSemaphore(%s,%d)\n", sem->name, curTask);

	// Return the semaphore Id
	return semId;
}

/**
 * _getNextSemaphoreId - get the Id of the next semaphore
 * @return: the new semaphore id
 *
 * This function returns the id of the next semaphore.  If the slot if taken
 * for the next logical id, increment id until an open slot is found.  This
 * should assert if the semaphoreArray is full.
 */
int _getNextSemaphoreId()
{
	assert("Semaphore Array is Full!" && (totalSemaphores < MAX_SEMAPHORES));
	assert("Error: Trying to call _getNextSemaphoreId in User Mode\n" && superMode);

	int currentId = nextSemaphoreId++;

	// Keep looping until there is an open slot
	while (semaphoreArray[currentId % MAX_SEMAPHORES] != NULL) {
		currentId = nextSemaphoreId++;
	}

	// Increment total counter
	totalSemaphores++;

	return currentId;
}

/**
 * _deleteSemaphore - delete a semaphore
 * @semId: the id of the semaphore to delete
 * @return: 1 if successfully deleted, 0 otherwise
 *
 * This function deletes a semaphore.
 */
bool _deleteSemaphore(int semId)
{
	assert("Error: Trying to call _deleteSemaphore in User Mode\n" && superMode);

	// Get the semaphore at that location
	Semaphore* sem = _getSemaphore(semId);

	// Make sure there is a semaphore at that location
	assert("There is no semaphore at that location" && (sem != NULL));

	if (KDEBUG) printf("deleteSemaphore(%s)\n", sem->name);

	// Delete any pending clock event
	_deleteClockEvent(semId);

	// Update information
	sem->type = -1;
	sem->taskNum = NO_PARENT;

	// Free all semaphore memory
	clearList(sem->blockedList);
	free(sem->blockedList);
	sem->blockedList = NULL;
	free(sem->name);
	free(sem);

	// Remove from slot
	semaphoreArray[semId % MAX_SEMAPHORES] = NULL;

	// Decrement totalSemaphore
	totalSemaphores--;

	return 1;
}

void _deleteAllSemaphores(void) {
	int i = 0;

	assert("Error: Trying to call _deleteAllSemaphores in User Mode\n" && superMode);
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphoreArray[i] == NULL) {
			continue;
		}
		_deleteSemaphore(i);
	}

    nextSemaphoreId = 0;
    totalSemaphores = 0;
}

void _deleteAllSemaphoresForTask(tid_t taskId) {
	int i = 0;
	Semaphore *sem;

	assert("Error: Trying to call _deleteAllSemaphoresForTask in User Mode\n" && superMode);

	// look for any semaphores created by this task and delete them
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		sem = semaphoreArray[i];

		// Make sure there is a semaphore at that location
		if (sem == NULL) {
			continue;
		}

		if (sem->taskNum == taskId) {
			// Delete any semaphores
			_deleteSemaphore(i);
		}
	}
}

/**
 * _getSemaphore - get a pointer to a semaphore
 * @id: a pointer to the semaphore to signal
 * @return: the pointer to the semaphore if in the list, 0 otherwise
 *
 * This function should return the pointer to the semaphore with the same
 * id of the input.  It should return a null pointer if the semaphore
 * is not in the list.
 */
Semaphore* _getSemaphore(int semId)
{
	assert("Calling _getSemaphore from user mode!" && superMode);

	// Get the semaphore at that location
	Semaphore* sem = semaphoreArray[semId % MAX_SEMAPHORES];

	// NULL is a valid return from _getSemaphore.  Callers must
	// check the return valid before using the semaphore.
	if (sem == NULL) {
		return NULL;
	}

    assert("Invalid semaphore reference\n" && sem->id == semId);
	return sem;
}

/**
 * _semSignal - signal semaphore s
 * @semId: the id of the semaphore to signal
 *
 * This function should signal a semaphore.  It should check to see if
 * there are tasks blocked on this semaphore.  If there are, the top
 * item should be unblocked and changed to state S_READY.  The state of the
 * semaphore should be changed depending on the type of the semaphore.
 */
void _semSignal(int semId)
{
}

/**
 * _semWait - wait on semaphore s
 * @semId: the id of the semaphore to wait on
 * @return: SEMNOTFOUND    if semID is not matched to any valid semaphore (_getSemaphore)
 *          WAITBLOCKED    if the task is blocked (the state of the task in its task 
 *                         control should also be set to S_BLOCKED in this case)
 *          WAITNOTBLOCKED if the task is not blocked on the call
 *
 * This function should wait on a semaphore until it is signaled. If the
 * semaphore has been signaled, the state of the semaphore should be
 * updated.  If it has not been signaled, the task should be blocked
 * and the state changed to represent that a task is blocked.
 */
int _semWait(int semId)
{
}

/**
 * _semTryLock - try to lock semaphore s
 * @semId: the id of the semaphore to try to lock
 * @return: 1 if task was signaled, 0 otherwise
 *
 * This function should attempt to change the state of the semaphore.
 * If it has not been signaled, it should return 0.  If it has been
 * signaled, the state of the semaphore should be changed and it should
 * return 1.
 */
int _semTryLock(int semId)
{
	return 1;
}

/**
 * _listSems: list the semaphores
 */
void _listSems()
{
	assert("Error: Trying to call _listSems in User Mode\n" && superMode);

	TCB* tcb = getTCB();
	Semaphore* sem;
	int i;

	for (i = MAX_TASKS; i < MAX_SEMAPHORES; i++) {
		// Get the semaphore at that location
		sem = semaphoreArray[i];

		// Make sure this is a valid semaphore
		if (sem == NULL) {
			continue;
		}

		// Print the semaphore information
		printf("\t%20s  %c  %d  ", sem->name, (sem->type?'C':'B'),sem->state);

		if (sem->taskNum != NO_PARENT && sem->taskNum != NO_TASK) {
			if (tcb[sem->taskNum].name == NULL) {
				printf("(kernel)");
			}
			else {
				printf("%s", tcb[sem->taskNum].name);
			}
		}
		printf("\n");
	}

	return;
}

//@ENABLE_SWAPS

