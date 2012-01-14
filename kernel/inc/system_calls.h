#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "type_defs.h"
#include "semaphores.h"

#define SWAP swapTask();

// System Calls
#define MAXSYSCALLS			20
#define GETTIDCALL			1
#define SWAPTASKCALL		2
#define SEMSIGNALCALL		3
#define SEMWAITCALL			4
#define LISTTASKSCALL		5
#define HELLOWORLDCALL		6
#define KILLTASKCALL		7
#define CREATETASKCALL		8
#define SIGKILLCALL			9
#define SIGACTIONCALL		10
#define SIGPROCMASKCALL		11
#define CREATESEMCALL		12
#define DELETESEMCALL		13
#define SEMTRYLOCKCALL		14
#define INSERTDELTACALL		15
#define DELETEDELTACALL		16
#define WAITTIDCALL			17
#define LISTSEMSCALL		18
#define LISTDELTACLOCKCALL	19
#define WAITCALL        	20


int helloWorldTask(int, char**);
tid_t gettid();
void swapTask();
void semSignal(int);
int semWait(int);
void listTasks();
void listSems();
void listDeltaClock();
Semaphore* helloWorld(int,char*,Semaphore*);
int killTask(int tid);
int createTask(NewTask*);
int sigKill(tid_t,int);
int sigAction(void (*sigHandler)(int), int sig);
int sigProcMask(int how, const sigmask_t *restrictMask);
int createSemaphore(char* name, int type, int state);
bool deleteSemaphore(int semId);
int semTryLock(int);
int insertDeltaClock(int time, int semId, int periodic);
int deleteClockEvent(int semId);
tid_t waittid(int task, int *stat_loc);
tid_t wait(int *stat_loc);

tid_t wait_b(int *stat_loc);

#endif
