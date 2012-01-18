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
	Node *node = malloc(sizeof(struct node));
	node->tid = tid;
	node->next = NULL;
	return node;
}

void clearNode(Node* node) {
	node->tid = -1;
	node->next = NULL;
	return;
}

QueueList* makeQueue(unsigned short priority) {
    QueueList* queue = malloc(sizeof(struct queueList));
    queue->priority = priority;
    queue->curNode = NULL;
    queue->lastNode = NULL;
    queue->nextList = NULL;
    return queue;
}


int removeNode(QueueList* list, tid_t tid) {
	Node* current = list->curNode;
	Node* prev = NULL;
	while(current != NULL){
		if(current->tid == tid){
			if(prev == NULL){
				list->curNode = current->next;
			} else{
				prev->next = current->next;
			}
			clearNode(current);
			return TRUE;
		} else {
			prev = current;
			current = current->next;
		}
	}
	return NO_TASK;
}

void enqueue(QueueList* list, tid_t tid) {
	Node* node = getNode(tid);
	if(list->lastNode == NULL){
		list->curNode = node;
	}else {
		list->lastNode->next = node;
	}
	list->lastNode = node;
    return;
}

int dequeue(QueueList* list){
    int tid;
    Node* current = list->curNode;
    if(current == NULL){
    	return NO_TASK;
    } else if(current->tid == list->lastNode->tid){
    	tid = current->tid;
    	list->curNode = NULL;
    	list->lastNode = NULL;
    } else {
    	tid = current->tid;
    	list->curNode = current->next;
    }
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
