#ifndef SYSTEM_CALLS_KERNEL_H
#define SYSTEM_CALLS_KERNEL_H

int U_gettid();
int U_swapTask();
int U_semSignal();
int U_semWait();
int U_listTasks();
int U_listSems();
int U_listDeltaClock();
int U_helloWorld();
int U_killTask();
int U_createTask();
int U_sigKill();
int U_sigAction();
int U_sigProcMask();
int U_createSemaphore();
int U_deleteSemaphore();
int U_semTryLock();
int U_insertDeltaClock();
int U_deleteClockEvent();
int U_waittid();
int U_wait();

#endif
