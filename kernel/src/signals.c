#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "signals.h"
#include "kernel.h"
#include "trap.h"
#include "semaphores.h"
#include "scheduler.h"

extern int superMode;					// system mode
extern int taskSemIds[MAX_TASKS];			// task semaphore
extern Semaphore* semaphoreList;		// linked list of active semaphores

//@DISABLE_SWAPS

int getSignoBit(int signo) {

	switch(signo) {
		case KSIGINT:
		{
			return KSIGINTBIT;
		}
		case KSIGKILL:
		{
			return KSIGKILLBIT;
		}
		case KSIGCHLD:
		{
			return KSIGCHLDBIT;
		}
		case KSIGCONT:
		{
			return KSIGCONTBIT;
		}
		case KSIGSTOP:
		{
			return KSIGSTOPBIT;
		}
		case KSIGTSTP:
		{
			return KSIGTSTPBIT;
		}
		default:
		{
			break;
		}
	}

	return -1;
}

// **********************************************************************
// **********************************************************************
// checkAndreceiveKSigKill
//
void checkAndreceiveKSigKill(int curTask)
{
	TCB* tcb = getTCB();

	if (tcb[curTask].name && tcb[curTask].signal)
	{
		if (tcb[curTask].signal & KSIGKILLBIT)
		{
			// ?? TODO:
			// Need to block KSIGKILL even though we should never
			// return.
			tcb[curTask].handlingSignal = TRUE;
			tcb[curTask].signal &= ~KSIGKILLBIT;
			(*tcb[curTask].sigKillHandler)(KSIGKILL);
			tcb[curTask].handlingSignal = FALSE;
		}
	}
}

void checkAndReceiveSignal(int signo,
						   int isDefault,
						   sigmask_t *p_mask,
						   void (*sigHandler)(int))
{
/* 	sigmask_t mask = 0; */
/* 	sigmask_t *p_mask = &mask; */
	sigmask_t sigBit = getSignoBit(signo);
	int curTask = _gettid();
	TCB* tcb = getTCB();

	int signal = tcb[curTask].signal & ~(tcb[curTask].signalMask) & sigBit;

	assert("PANIC: bad handler" && sigHandler);

	if (!signal) {
		return;
	}

	sigEmptySet(p_mask);
	sigAddSet(p_mask, signo);
	_sigProcMask(KSIG_BLOCK, p_mask);
	tcb[curTask].handlingSignal = TRUE;
	tcb[curTask].signal &= ~sigBit;

	// If it is the default handler
	if (isDefault) {
		// Call it in kernel mode
		(*sigHandler)(signo);

		// Unblock a signal that was being handled
		tcb[curTask].handlingSignal = FALSE;
		_sigProcMask(KSIG_UNBLOCK, p_mask);

	} else {
		// Point system_call_params to valid memory memory location
		// for the curTask
		system_call_params = tcb[curTask].system_call_params;

		// Assign the signal handler to trap_union
		// Global variable declared in kernel.c used by trap
		system_call_params->signal_handler = sigHandler;

		// Assign superMode
		superMode = FALSE;

		// longjmp to user context with a value signifying a signal
		// handler is ready
		longjmp(tcb[curTask].context, signo);
	}
}

// **********************************************************************
// **********************************************************************
//	sigAction - install signal handler for a specified KSIGNAL
//
//  sigHandler = pointer to signal handler function
//  sig = signal
//
int _sigAction(void (*sigHandler)(int), int sig)
{
	TCB* tcb = getTCB();
	int curTask = _gettid();

	assert("Error: Trying to call _sigAction in User Mode\n" && superMode);

	switch (sig)
	{
		case KSIGINT:
		{
			tcb[curTask].sigIntHandler = sigHandler;		// KSIGINT handler
			return 0;
		}
		case KSIGCHLD:
		{
			tcb[curTask].sigChldHandler = sigHandler;		// KSIGCHLD handler
			return 0;
		}
		case KSIGTSTP:
		{
			tcb[curTask].sigTstpHandler = sigHandler;		// KSIGSTP handler
			return 0;
		}
	}
	return -1;
}


// **********************************************************************
//	sigKill - send signal to task(s)
//
//	taskId
//    >=0: send signal to the specified task
//   <0: send signal to specified task and everyone in its group
extern jmp_buf reset_context; // Defined in kernel.c
int _sigKill(int taskId, int sig)
{
	int i = 0;
	int retVal = 1;
	TCB* tcb = getTCB();

	assert("Error: Trying to call _sigKill in User Mode\n" && superMode);

	sigmask_t sigMask = getSignoBit(sig);

	if (KDEBUG) {
		printf ("sigKill(%d,%d) --- (%#x)\n", taskId, sig, sigMask);
	}

	if (taskId == NO_PARENT) {
		return 0;
	}

	if (taskId >= MAX_TASKS) {
		return -1;
	}

	if (taskId == 0) {
		if (sig == KSIGKILL || sig == KSIGSTOP) {
			longjmp(reset_context, POWER_DOWN_QUIT);
		}
	}

	// check for task
	//printf("tcb[%d] = %x\n", taskId, &tcb[taskId]);
	//printf("taskId = %d with name %s, and state %d\n", taskId, tcb[taskId].name, tcb[taskId].state);
	if ((taskId >= 0) && tcb[taskId].name) {

		// Add the new signal to the task block
		tcb[taskId].signal |= sigMask;

		// If the task is stopped or if the signal is KSIGKILL, wake the task up to handle it
		if (tcb[taskId].state == S_STOPPED || sig == KSIGKILL) {

			// If the task is on a blocked queue, remove it
			if (tcb[taskId].event) {
				if ((removeNode(tcb[taskId].event->blockedList, taskId) == NO_TASK) && KDEBUG) {
					printf ("WARNING: task %d not found in the blocked list for %s during _sigKill\n", taskId, tcb[taskId].event->name);
				}
				tcb[taskId].event = NULL;
			}
			// Add the task to the ready queue
			if (tcb[taskId].state != S_READY) {
                reschedule(taskId);
			}
		}
		retVal = 0;
	} else if (taskId < 0) {
		taskId = -taskId;
		for (i=0; i<MAX_TASKS; i++)
		{
			if (tcb[i].name && tcb[i].tgid == tcb[taskId].tgid) {
				tcb[i].signal |= sigMask;
				if (tcb[i].state == S_STOPPED || sig == KSIGKILL) {
					if (tcb[i].event) {
						if ((removeNode(tcb[i].event->blockedList, i) == NO_TASK) && KDEBUG) {
							printf ("WARNING: task %d not found in the blocked list for %s during _sigKill\n", taskId, tcb[taskId].event->name);
						}

						tcb[i].event = NULL;
					}
					if (tcb[i].state != S_READY) {
						reschedule(i);
					}
				}

			}
		}
		retVal = 0;
	}

	// error
	return retVal;
}


/**
    Returns -1 if specified task is not a child of
    calling task.  Returns 0 if
    child not stopped or terminated.  Returns child tid
    if child is stopped or terminated.
*/
tid_t _waittid(int pTask, int childTask, int *stat_loc)
{
	int hasChild = 0;
	TCB* tcb = getTCB();

	if (tcb[childTask].name == NULL) {
	    return -1;
	}

    if (tcb[childTask].parent == NO_PARENT && tcb[childTask].state == S_ZOMBIE) {
        _killTask(childTask);
        return -1;
    }

    if (tcb[childTask].parent != pTask) {
        return -1;
    }

    hasChild = TRUE;

    if (_semTryLock(taskSemIds[childTask]) == 0) {
        return 0; // child not finished
    }

    if (stat_loc) {
        *stat_loc = tcb[childTask].result;
    }

    if (tcb[childTask].state == S_ZOMBIE) {
        _killTask(childTask);
        return childTask; // child finished
    }

    if (tcb[childTask].state == S_STOPPED) {
        return childTask;  // child stopped
    }

	return(hasChild ? 0 : -1);
}

/**
    Returns -1 if no child of calling task exists.
    Returns 0 if child not stopped or terminated.
    Returns child tid if child is stopped or terminated.
*/
tid_t _wait(int taskId, int *stat_loc)
{
	int hasChild = 0;
	int i = 0;
	TCB* tcb = getTCB();

	for (i = 0 ; i < MAX_TASKS ; ++i) {

		// Not a valid entry.  Next!
		if (tcb[i].name == NULL) {
			continue;
		}

		// Clean any orphaned tasks as a public service for kernel
		// house cleaning.  Next!
		if (tcb[i].parent == NO_PARENT && tcb[i].state == S_ZOMBIE) {
			_killTask(i);
			continue;
		}

		// Not my kids.  Not my problem.  Next!
		if (tcb[i].parent != taskId) {
			continue;
		}

		hasChild = TRUE;

		if (_semTryLock(taskSemIds[i]) == 0) {
			continue;
		}

		if (stat_loc) {
			*stat_loc = tcb[i].result;
		}

		if (tcb[i].state == S_ZOMBIE) {
			_killTask(i);
			return i;
		}

		if (tcb[i].state == S_STOPPED) {
			return i;
		}
	}
	return(hasChild ? 0 : -1);
}


//	WIFKSTOPPED
//
//  Returns TRUE is status indicates the process is stopped. Otherwise
//  returns false.
int WIFKSTOPPED(int status)
{
	return (status & KSIGSTOPBIT || status & KSIGTSTPBIT);
}

// **********************************************************************
// **********************************************************************
//	WIFKSIGNALED
//
//  Returns TRUE is status indicates the process terminated from a
//  signal. Otherwise returns false.
int WIFKSIGNALED(int status)
{
	return (status & KSIGINTBIT || status & KSIGKILLBIT);
}

// **********************************************************************
// **********************************************************************
//	WIFKEXITED
//
//  Returns TRUE is status indicates the process exited
//  normally. Otherwise returns false.
int WIFKEXITED(int status)
{
	return (!WIFKSIGNALED(status) && !WIFKSTOPPED(status));
}

// **********************************************************************
// **********************************************************************
//	WKEXITSTATUS
//
//  Returns the exit value form the process (i.e., the return value)
int WKEXITSTATUS(int status)
{
	return ((status >> 24) & 0x000000ff);
}

// **********************************************************************
// **********************************************************************
//	WSTOPKSIG
//
//  Returns the signal number that stopped the task (KSIGSTOP or
//  KSIGTSTP).
int WSTOPKSIG(int status)
{
	if (status & KSIGTSTPBIT) {
		return KSIGTSTP;
	}
	if (status & KSIGSTOPBIT) {
		return KSIGSTOP;
	}
	return 0;
}

// **********************************************************************
// **********************************************************************
//	WTERMKSIG
//
//  Returns the signal number that killed the task (KSIGINT or
//  KSIGKILL).
int WTERMKSIG(int status)
{
	if (status & KSIGKILLBIT) {
		return KSIGKILL;
	}
	if (status & KSIGINTBIT) {
		return KSIGINT;
	}
	return 0;
}
// **********************************************************************
// **********************************************************************
// sigEmptySet
//
// Clears all signals from the set.
int sigEmptySet(sigmask_t *set)
{
	if (set == NULL){
		return -1;
	}
	*set = 0;
	return 0;
}

// **********************************************************************
// **********************************************************************
// sigAddSet
//
// Adds signo to the set.  Note there is a mapping between signo and
// its actual bit location in a bit vector.  For example, KSIGCHLD
// is at bit location KSIGCHLDBIT.
int sigAddSet(sigmask_t *set, int signo)
{
	if (set == NULL) {
		return -1;
	}
	switch(signo) {
		case KSIGINT:
		{
			*set |= KSIGINTBIT;
			break;
		}
		case KSIGKILL:
		{	// cannot be set
			break;
		}
		case KSIGCHLD:
		{
			*set |= KSIGCHLDBIT;
			break;
		}
		case KSIGCONT:
		{	// cannot be set
			break;
		}
		case KSIGSTOP:
		{   // cannot be set
			break;
		}
		case KSIGTSTP:
		{
			*set |= KSIGTSTPBIT;
			break;
		}
		default:
		{
			return -1;
		}
	}
	return 0;
}

// **********************************************************************
// **********************************************************************
// sigProcMask
int _sigProcMask(int how, const sigmask_t *restrict)
{
	TCB* tcb = getTCB();
	int curTask = _gettid();

    assert("Error: Trying to call _sigProcMask in User Mode\n" && superMode);
	assert("PANIC: not a valid task" && tcb[curTask].name);

	if (restrict == NULL || (how != KSIG_UNBLOCK && how != KSIG_BLOCK)) {
		return -1;
	}

	// Clear the bits in signalMask according to restrict
	if (how == KSIG_UNBLOCK) {
		tcb[curTask].signalMask &= ~(*restrict);
	}

	// Set bits in signalMask according to restrict
	if (how == KSIG_BLOCK) {
		tcb[curTask].signalMask |= (*restrict);
	}

	return 0;
}

// **********************************************************************
// **********************************************************************
//	KSigKillHandler
//
void KSigKillHandler(int sig)
{
	int sigBit = getSignoBit(sig);
	TCB* tcb = getTCB();
	int curTask = _gettid();

	assert("PANIC: signal received by defunct task" && tcb[curTask].name);
	assert("PANIC: bad signal bit" && sigBit != -1);

	tcb[curTask].result = sigBit;
	longjmp(tcb[curTask].kill_context, TRUE);
}

// **********************************************************************
// **********************************************************************
//	defaultKSigIntHandler
//
//  The default behavior is to terminate the running task.
void defaultKSigIntHandler(int sig)
{
	TCB* tcb = getTCB();
	int curTask = _gettid();

	// ASSUMPTION: only called from valid S_RUNNING
	assert("PANIC: signal received by task not in S_RUNNING" &&
		   (tcb[curTask].state == S_RUNNING || curTask == 0));
	KSigKillHandler(sig);
}

// **********************************************************************
// **********************************************************************
//	defaultKSigChldHandler
//
//  Default behavior is to do nothing.
void defaultKSigChldHandler(int sig)
{
	TCB* tcb = getTCB();
	int curTask = _gettid();

	// ASSUMPTION: only called from valid S_RUNNING
	assert("PANIC: KSIGCHLD received by task not in S_RUNNING" &&
		   tcb[curTask].state == S_RUNNING);
	assert("PANIC: KSIGCHLD received by defunct task" &&
		   tcb[curTask].name);
	// Default behavior is to do nothing
}

// **********************************************************************
// **********************************************************************
//	KSigConthandler
//
//  KSIGCONT behavior cannot be overridden!  Must move a task from
//  S_STOPPED to S_READY when recieved. (i.e., this function is
//  invoked from the dispatcher).  Note that you can only stop ready tasks
//  so it is safe to move a task from S_STOPPED to S_READY.
void KSigContHandler(int sig)
{
	TCB* tcb = getTCB();
	int curTask = _gettid();

	// ASSUMPTION: only called from valid task in S_RUNNING or S_STOPPED
	assert("PANIC: KSIGCONT received by task not in S_STOPPED or S_RUNNING" &&
		   (tcb[curTask].state == S_STOPPED || tcb[curTask].state == S_RUNNING));
	assert("PANIC: stop received by defunct task" &&
		   tcb[curTask].name);

	tcb[curTask].state = S_READY;
	reschedule(curTask);
}

// **********************************************************************
// **********************************************************************
//	KSigStopHandler
//
//  KSIGTOP behavior cannot be overridden!  Must stop a task when the
//  signal is received (i.e., this function is invoked from the
//  dispatcher).
void KSigStopHandler(int sig)
{
	int sigBit = getSignoBit(sig);
	TCB* tcb = getTCB();
	int curTask = _gettid();

	// ASSUMPTION: only called from valid task in S_RUNNING
	assert("PANIC: stop received by task not in S_RUNNING" &&
		   (tcb[curTask].state == S_RUNNING || tcb[curTask].state == S_READY));
	assert("PANIC: stop received by defunct task" &&
		   tcb[curTask].name);
	assert("PANIC: bad signal bit" && sigBit != -1);

	// We are using the taskSemIds to indicate if a child has
	// been reaped. Signaling means it needs to be reaped.
	// We follow the same protocol in killTask();

	tcb[curTask].state = S_STOPPED;
	tcb[curTask].result = sigBit;
	if (tcb[curTask].parent != NO_PARENT) {
		_semSignal(taskSemIds[curTask]);
		_sigKill(tcb[curTask].parent, KSIGCHLD);

	}
}

// **********************************************************************
// **********************************************************************
//	defaultKSigTstpHandler
//
//  KSIGTSTP by default stops a task when the signal is received
//  (i.e., this function is invoked from the dispatcher).
void defaultKSigTstpHandler(int sig)
{
	KSigStopHandler(sig);
}

//@ENABLE_SWAPS
