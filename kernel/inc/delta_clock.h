#ifndef DELTA_CLOCK_H
#define DELTA_CLOCK_H

#include "semaphores.h"

#define ONE_TENTH_SEC		(CLOCKS_PER_SEC/10)
static const int PERIODIC = 1;
static const int NOT_PERIODIC = 0;

int _insertDeltaClock(int time, int semId, int periodic);
int _deleteClockEvent(int semId);
int _tickDeltaClock();
int _listDeltaClock();

#endif
