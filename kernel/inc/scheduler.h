#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "queue_list.h"

#define NO_TASK -1

void initScheduler();
int scheduler();
void reschedule(int);
void deschedule(int task);
void clearScheduler();
QueueList* getFreeList();


#endif
