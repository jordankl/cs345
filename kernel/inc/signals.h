#ifndef SIGNALS_H
#define SIGNALS_H

#include "type_defs.h"
#include "trap.h"

// Signals
#define KSIGINT	    2
#define KSIGKILL	9
#define KSIGCHLD    17
#define KSIGCONT	18
#define KSIGSTOP	19
#define KSIGTSTP	20

#define KSIGINTBIT  1 << (KSIGINT - 1)
#define KSIGKILLBIT	1 << (KSIGKILL - 1)
#define KSIGCHLDBIT 1 << (KSIGCHLD - 1)
#define KSIGCONTBIT 1 << (KSIGCONT - 1)
#define KSIGSTOPBIT 1 << (KSIGSTOP - 1)
#define KSIGTSTPBIT 1 << (KSIGTSTP - 1)

#define KSIG_UNBLOCK 0
#define KSIG_BLOCK	1

extern trap_struct* system_call_params;  // declared in kernel.c

void checkAndReceiveSignal(int,
						   int,
						   sigmask_t *,
						   void (*sigHandler)(int));
void checkAndreceiveKSigKill(int);
int _sigAction(void (*sigHandler)(int), int sig);
int _sigKill(tid_t taskId, int sig);

tid_t _waittid(int pTask, int childTask, int *stat_loc);
tid_t _wait(int task, int *stat_loc);

int WIFKSTOPPED(int status);
int WIFKSIGNALED(int status);
int WIFKEXITED(int status);
int WKEXITSTATUS(int status);
int WSTOPKSIG(int status);
int WTERMKSIG(int status);

int sigEmptySet(sigmask_t *set);
int sigAddSet(sigmask_t *set, int signo);
int _sigProcMask(int how, const sigmask_t *restrict);

// Handlers
void KSigKillHandler(int);
void defaultKSigIntHandler(int);
void defaultKSigChldHandler(int);
void KSigContHandler(int);
void KSigStopHandler(int);
void defaultKSigTstpHandler(int);

#endif
