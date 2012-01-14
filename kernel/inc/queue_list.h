#ifndef QUEUE_LIST_H
#define QUEUE_LIST_H

#include "type_defs.h"

typedef struct node {
	tid_t tid;
	struct node* next;
} Node;

typedef struct queueList {
	Node *curNode;  // HEAD where you dequeue
	Node *lastNode; // TAIL where you enqueue
	struct queueList *nextList;
	unsigned short priority;
} QueueList;

/** getNode:
 * returns the Node structure associated with the passed in task ID.
 * In this implementation, there is exactly one node for each task
 * stored in a global array defined in the C-file.
 */
Node* getNode(tid_t tid);

/** clearNode:
 * NULLs out any information in the node
 */
void clearNode(Node*);

/** makeQueue
 * Mallocs a new QueueList, initializes the fields to 0,
 * and sets the priority to the passed in parameter.
 */
QueueList* makeQueue(unsigned short priority);

/** removeNode:
 * Searches the passed in queue for the node matching the passed in
 * task ID.  If the node is found, then it is removed from the queue,
 * and the node itself is cleared (clearNode).
 *
 * Return NO_TASK if the task is not found in the queue
 * Return TRUE if it is found and cleared
 * Return FALSE never
 *
 */
int removeNode(QueueList*, tid_t);

/** enqueueNode:
 * Enqueue the passed in task ID.  Use the getNode function to obtain
 * an actual node to insert into the list.  The end of the queue is
 * the "lastNode"
 */
void enqueue(QueueList*, tid_t);

/** dequeue:
 * dequeue form the queue.  The curNode and lastNode are intended to
 * help impement this function.  The head of the queue the
 * curNode. Dequeue from the head of the list.
 *
 * Return NO_TASK if the head of the queue is NULL (curNode)
 * Return tid for the current head
 *
 * The queue should be modified after the call to have a new head.
 */
int dequeue(QueueList*);

/** clearList:
 * Removes any entries from the queue in list.
 */
void clearList(QueueList *list);

/** clearListNode
 * Nulls out anything in a QueueList structure.
 */
void clearListNode(QueueList *list);

#endif
