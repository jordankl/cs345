#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "system_calls_kernel.h"
#include "trap.h"
#include "kernel.h"
#include "semaphores.h"
#include "signals.h"
#include "delta_clock.h"

extern trap_struct* system_call_params;

/* Return 1 if you should return immediately, 0 to re=schedule */

//@DISABLE_SWAPS

int U_gettid()
{
	int rv = 0;

	_setTrapParamsContext(system_call_params);

	// Do call
	rv = _gettid();
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_tid = rv;

	// Return and swap
	return 1;
}

int U_swapTask()
{
	_setTrapParamsContext(system_call_params);
	// Do System Call
	_swapTask();
	system_call_params = _getTrapParamsContext();

	return 1;
}

int U_semSignal()
{
	// Unpack Variables
	int semId = *((int*)system_call_params->params[1]);

	_setTrapParamsContext(system_call_params);
	// Do system call
	_semSignal(semId);
	system_call_params = _getTrapParamsContext();

	// Return and swap
	return 1;
}

int U_semWait()
{
	// Unpack Variables
	int semId = *((int*)system_call_params->params[1]);
	int returnInt = 0;

	_setTrapParamsContext(system_call_params);
	// Assign return value
	returnInt = _semWait(semId);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = returnInt;

	// Return
	return (!returnInt);
}

int U_listTasks()
{
	_setTrapParamsContext(system_call_params);
	_listTasks();
	system_call_params = _getTrapParamsContext();

	return 1;
}

int U_listSems()
{
	_setTrapParamsContext(system_call_params);
	_listSems();
	system_call_params = _getTrapParamsContext();

	return 1;
}

int U_listDeltaClock()
{
	_setTrapParamsContext(system_call_params);
	_listDeltaClock();
	system_call_params = _getTrapParamsContext();

	return 1;
}

int U_helloWorld()
{
	// Unpack Variables
	int num = *((int*)system_call_params->params[1]);
	char* phrase = (char*)system_call_params->params[2];
	Semaphore* sem = (Semaphore*)system_call_params->params[3];
	void* rv = NULL;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = (void*)_helloWorld(num, phrase, sem);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_pointer = rv;

	return 1;
}

int U_killTask()
{
	// Unpack Variable
	int tid = *((int*)system_call_params->params[1]);
	int rv = 0;
	
	assert("Error: calling killTask on yourself\n" && tid != _gettid());

	_setTrapParamsContext(system_call_params);
	// Do System Call
	rv = _killTask(tid);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	return 1;
}

int U_createTask()
{
	// Unpack Variables
	NewTask* newTask = (NewTask*)system_call_params->params[1];

	_setTrapParamsContext(system_call_params);
	// Do system call
	// _createTask does not return.  It invokes initTask on 
	// a new stack for the new task, and it returns directly 
	// to the trap call for the caller.  The return value
	// is set in the trap call. 
	_createTask(newTask);

	// Return
	return 1;
}

int U_sigKill()
{
	// Unpack Variables
	tid_t taskId = *((tid_t*)system_call_params->params[1]);
	int sig = *((int*)system_call_params->params[2]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _sigKill(taskId, sig);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_sigAction()
{
	// Unpack Variables
	void (*sigHandler)(int) = (void ( * )(int)) system_call_params->params[1];
	int sig = *((int*)system_call_params->params[2]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _sigAction(sigHandler, sig);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_sigProcMask()
{
	int how = *((int*)system_call_params->params[1]);
	const sigmask_t* restrictMask = (const sigmask_t*)system_call_params->params[2];
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _sigProcMask(how, restrictMask);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_createSemaphore()
{
	// Unpack Variables
	char* name = (char*)system_call_params->params[1];
	int type = *((int*)system_call_params->params[2]);
	int state = *((int*)system_call_params->params[3]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _createSemaphore(name, type, state);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv; 

	// Return
	return 1;
}

int U_deleteSemaphore()
{
	// Unpack Variables
	int semId = *((int*)system_call_params->params[1]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _deleteSemaphore(semId);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_semTryLock()
{
	// Unpack Variables
	int semId = *((int*)system_call_params->params[1]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _semTryLock(semId);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_insertDeltaClock()
{
	// Unpack Variables
	int time = *((int*)system_call_params->params[1]);
	int semId = *((int*)system_call_params->params[2]);
	int periodic = *((int*)system_call_params->params[3]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _insertDeltaClock(time,semId,periodic);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_deleteClockEvent()
{
	// Unpack Variables
	int semId = *((int*)system_call_params->params[1]);
	int rv = 0;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _deleteClockEvent(semId);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_int = rv;

	// Return
	return 1;
}

int U_waittid()
{
	// Unpack Variables
	int pTask = *((int*)system_call_params->params[1]);
	int task = *((int*)system_call_params->params[2]);
	int* stat_loc = (int*)system_call_params->params[3];
	tid_t rv = NO_TASK;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _waittid(pTask, task, stat_loc);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_tid = rv;

	// Return
	return 1;
}

int U_wait()
{
	// Unpack Variables
	int task = *((int*)system_call_params->params[1]);
	int* stat_loc = (int*)system_call_params->params[2];
	tid_t rv = NO_TASK;

	_setTrapParamsContext(system_call_params);
	// Do system call
	rv = _wait(task, stat_loc);
	system_call_params = _getTrapParamsContext();
	system_call_params->return_value.return_tid = rv;

	// Return
	return 1;
}

//@ENABLE_SWAPS
