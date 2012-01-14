// @DISABLE_SWAPS
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include "queue_list.h"
#include "scheduler.h"
#include "kernel.h"

// Create an entry for every possible task.  We recycle these entries
// as a task cannot appear in more than one queue at any time
Node tasks[MAX_TASKS];

Node* getNode(tid_t tid) {
}

void clearNode(Node* node)
{
	return;
}

QueueList* makeQueue(unsigned short priority) {
    QueueList* queue = NULL; 
    return queue;
}


int removeNode(QueueList* list, tid_t tid)
{
	return NO_TASK;
}

void enqueue(QueueList* list, tid_t tid)
{
    return;
}

int dequeue(QueueList* list)
{
    int tid;

    return tid;
}


void clearList(QueueList *list)
{
	return;
}

void clearListNode(QueueList *list)
{
	return;
}

// @ENABLE_SWAPS
