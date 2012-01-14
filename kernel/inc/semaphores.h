#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include "type_defs.h"
#include "queue_list.h"

#define MAX_SEMAPHORES	1000
#define NULL_SEMAPHORE	-1

// Semaphore Types
static const int BINARY = 0;
static const int COUNTING = 1;

static const int WAITNOTBLOCKED = 0;
static const int WAITBLOCKED = 1;
static const int SEMNOTFOUND = 2;

/**
 * struct Semaphore
 * @id: the if of the semaphore
 * @name: the name of the semaphore
 * @state: the state (or value) of the semaphore
 * @type: the type (binary or counting) of semaphore
 * @taskNum: the task ID of the task that created this semaphore
 * @blockedList: a queueList of tasks blocked on this semaphore
 */
typedef struct
{
	int id;
	char* name;
	int state;
	int type;
	int taskNum;
	struct queueList *blockedList;
} Semaphore;

// Semaphore Functions
int _createSemaphore(char* name, int type, int state);
int _getNextSemaphoreId();
bool _deleteSemaphore(int);
void _deleteAllSemaphores(void);
void _deleteAllSemaphoresForTask(tid_t taskId);
Semaphore* _getSemaphore(int);
void _semSignal(int);
int _semWait(int);
int _semTryLock(int);
void _listSems();

#endif
