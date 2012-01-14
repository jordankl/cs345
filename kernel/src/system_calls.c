#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "system_calls.h"
#include "trap.h"
#include "kernel.h"

extern int superMode;					// system mode
extern int taskSemIds[MAX_TASKS];			// task semaphore

/* System Calls */
tid_t gettid()
{
	// Make parameters structure
	trap_struct getTidParams;
	jmp_buf buf;
	void* local_params[1] = { (void*)&buf };
	getTidParams.params = local_params;

    assert("Error: Making system call to gettid from public interface when already in Kernel Mode (use _gettid instead)" && !superMode);

	// Call trap
	trap(&getTidParams,GETTIDCALL);

	// Get return value
	return getTidParams.return_value.return_tid;
}

//@DISABLE_SWAPS
void swapTask()
{
	// Make parameters structure
	trap_struct swapTaskParams;
	jmp_buf buf;
	void* local_params[1] = { (void*)&buf };
	swapTaskParams.params = local_params;

    assert("Error: Making system call to swapTask from public interface when already in Kernel Mode (use _swapTask instead)" && !superMode);

	// Call trap
	trap(&swapTaskParams,SWAPTASKCALL);

	// Return to calling function
	return;
}
//@ENABLE_SWAPS

void semSignal(int semId)
{
	// Make parameters structure
	trap_struct semSignalParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)&buf, (void*)&semId };
	semSignalParams.params = local_params;

    assert("Error: Making system call to semSignal from public interface when already in Kernel Mode (use _semSignal instead)" && !superMode);

	// Call trap
	trap(&semSignalParams,SEMSIGNALCALL);

	// Return
	return;
}

int semWait(int semId)
{
	// Make parameters structure
	trap_struct semWaitParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)&buf, (void*)&semId };
	semWaitParams.params = local_params;

    assert("Error: Making system call to semWait from public interface when already in Kernel Mode (use _semWait instead)" && !superMode);

	// Call trap
	trap(&semWaitParams,SEMWAITCALL);

	// Get return value
	return semWaitParams.return_value.return_int;
}

void listTasks()
{
	// Make parameters structure
	trap_struct listTasksParam;
	jmp_buf buf;
	void* local_params[1] = { (void*)buf };
	listTasksParam.params = local_params;

	assert("Error: Making system call to listTasks from public interface when already in Kernel Mode (use _listTasks instead)" && !superMode);

	// Call trap
	trap(&listTasksParam,LISTTASKSCALL);

	// Return to calling function
	return;
}

void listSems()
{
	// Make parameters structure
	trap_struct listSemsParam;
	jmp_buf buf;
	void* local_params[1] = { (void*)buf };
	listSemsParam.params = local_params;

    assert("Error: Making system call to listSems from public interface when already in Kernel Mode (use _listSems instead)" && !superMode);

	// Call trap
	trap(&listSemsParam,LISTSEMSCALL);

	// Return to calling function
	return;
}

void listDeltaClock()
{
	// Make parameters structure
	trap_struct listDeltaClockParam;
	jmp_buf buf;
	void* local_params[1] = { (void*)buf };
	listDeltaClockParam.params = local_params;

    assert("Error: Making system call to listDeltaClock from public interface when already in Kernel Mode (use _listDeltaClock instead)" && !superMode);

	// Call trap
	trap(&listDeltaClockParam,LISTDELTACLOCKCALL);

	// Return to calling function
	return;
}

Semaphore* helloWorld(int num, char* phrase, Semaphore* sem)
{
	// Make parameters structure
	trap_struct helloWorldParams;
	jmp_buf buf;
	void* local_params[4] = { (void*)buf, (void*)&num, (void*)phrase, (void*)sem };
	helloWorldParams.params = local_params;

    assert("Error: Making system call to helloWorld from public interface when already in Kernel Mode (use _helloWorld instead)" && !superMode);

	// Call trap
	trap(&helloWorldParams,HELLOWORLDCALL);

	// Get return value
	return (Semaphore*)helloWorldParams.return_value.return_pointer;
}

int killTask(int tid)
{
	// Make parameters structure
	trap_struct killTaskParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)buf, (void*)&tid };
	killTaskParams.params = local_params;

    assert("Error: Making system call to killTask from public interface when already in Kernel Mode (use _killTask instead)" && !superMode);

	// Call trap
	trap(&killTaskParams,KILLTASKCALL);

	// Get return value
	return killTaskParams.return_value.return_int;
}

int createTask(NewTask* taskInfo)
{
	// Make parameters structure
	trap_struct createTaskParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)buf, (void*)taskInfo };
	createTaskParams.params = local_params;

    assert("Error: Making system call to createTask from public interface when already in Kernel Mode (use _createTask instead)" && !superMode);

	// Call trap
	trap(&createTaskParams,CREATETASKCALL);

	// Get return value
	return createTaskParams.return_value.return_int;
}

int sigKill(tid_t taskId, int sig)
{
	// Make parameters structure
	trap_struct sigKillParams;
	jmp_buf buf;
	void* local_params[3] = { (void*)buf, (void*)&taskId, (void*)&sig };
	sigKillParams.params = local_params;

    assert("Error: Making system call to sigKill from public interface when already in Kernel Mode (use _sigKill instead)" && !superMode);

	// Call trap
	trap(&sigKillParams,SIGKILLCALL);

	// Get return value
	return sigKillParams.return_value.return_int;
}

int sigAction(void (*sigHandler)(int), int sig)
{
	// Make parameters structure
	trap_struct sigActionParams;
	jmp_buf buf;
	void* local_params[3] = { (void*)buf, sigHandler, (void*)&sig };
	sigActionParams.params = local_params;

    assert("Error: Making system call to sigAction from public interface when already in Kernel Mode (use _sigAction instead)" && !superMode);

	// Call trap
	trap(&sigActionParams,SIGACTIONCALL);

	// Get return value
	return sigActionParams.return_value.return_int;
}

//@DISABLE_SWAPS
int sigProcMask(int how, const sigmask_t *restrictMask)
{
	// Make parameters structure
	trap_struct sigProcMaskParams;
	jmp_buf buf;
	void* local_params[3] = { (void*)buf, (void*)&how, (void*)restrictMask };
	sigProcMaskParams.params = local_params;

    assert("Error: Making system call to sigProcMask from public interface when already in Kernel Mode (use _sigProcMask instead)" && !superMode);

	// Call trap
	trap(&sigProcMaskParams,SIGPROCMASKCALL);

	// Get return value
	return sigProcMaskParams.return_value.return_int;
}
//@ENABLE_SWAPS

int createSemaphore(char* name, int type, int state)
{
	// Make parameters structure
	trap_struct createSemParams;
	jmp_buf buf;
	void* local_params[4] = { (void*)buf, (void*)name, (void*)&type, (void*)&state };
	createSemParams.params = local_params;

	assert("Error: Making system call to createSemaphore from public interface when already in Kernel Mode (use _createSemaphore instead)" && !superMode);

	// Call trap
	trap(&createSemParams,CREATESEMCALL);

	// Get return value
	return createSemParams.return_value.return_int;
}

bool deleteSemaphore(int semId)
{
	// Make parameters structure
	trap_struct deleteSemParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)buf, (void*)&semId };
	deleteSemParams.params = local_params;

    assert("Error: Making system call to deleteSemaphore from public interface when already in Kernel Mode (use _deleteSemaphore instead)" && !superMode);

	// Call trap
	trap(&deleteSemParams,DELETESEMCALL);

	// Get return value
	return (deleteSemParams.return_value.return_int != 0);
}

int semTryLock(int semId)
{
	// Make parameters structure
	trap_struct semTryLockParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)buf, (void*)&semId };
	semTryLockParams.params = local_params;

    assert("Error: Making system call to semTryLock from public interface when already in Kernel Mode (use _semTryLock instead)" && !superMode);

	// Call trap
	trap(&semTryLockParams,SEMTRYLOCKCALL);

	// Get return value
	return semTryLockParams.return_value.return_int;
}

int insertDeltaClock(int time, int semId, int periodic)
{
	// Make parameters structure
	trap_struct insertDeltaParams;
	jmp_buf buf;
	void* local_params[4] = { (void*)buf, (void*)&time, (void*)&semId, (void*)&periodic };
	insertDeltaParams.params = local_params;

    assert("Error: Making system call to insertDeltaClock from public interface when already in Kernel Mode (use _insertDeltaClock instead)" && !superMode);

	// Call trap
	trap(&insertDeltaParams,INSERTDELTACALL);

	// Get return value
	return insertDeltaParams.return_value.return_int;
}

int deleteClockEvent(int semId)
{
	// Make parameters structure
	trap_struct deleteClockParams;
	jmp_buf buf;
	void* local_params[2] = { (void*)buf, (void*)&semId };
	deleteClockParams.params = local_params;

    assert("Error: Making system call to deleteClockEvent from public interface when already in Kernel Mode (use _deleteClockEvent instead)" && !superMode);

	// Call trap
	trap(&deleteClockParams,DELETEDELTACALL);

	// Get return value
	return deleteClockParams.return_value.return_int;
}

tid_t waittid(int task, int *stat_loc)
{
	// Make parameters structure
	trap_struct waitTidClockParams;
	jmp_buf buf;
	int pTask = gettid();
	void* local_params[4] = { (void*)buf, (void*)&pTask, (void*)&task, (void*)stat_loc };
	waitTidClockParams.params = local_params;

	assert("Error: Making system call to waittid from public interface when already in Kernel Mode (use _waittid instead)" && !superMode);

	// Call trap
	trap(&waitTidClockParams,WAITTIDCALL);

	// Get return value
	return waitTidClockParams.return_value.return_int;
}

tid_t wait(int *stat_loc)
{
	// Make parameters structure
	trap_struct waitClockParams;
	jmp_buf buf;
	int task = gettid();
	void* local_params[3] = { (void*)buf, (void*)&task, (void*)stat_loc };
	waitClockParams.params = local_params;

	assert("Error: Making system call to wait from public interface when already in Kernel Mode (use _wait instead)" && !superMode);

	// Call trap
	trap(&waitClockParams,WAITCALL);

	// Get return value
	return waitClockParams.return_value.return_int;
}

// Blocking wait call.  Relies on wait()
tid_t wait_b(int *stat_loc) {
	int curTask = gettid();
	while (semTryLock(taskSemIds[curTask]) == 0) {
		swapTask();
	}

	return wait(stat_loc);
}
/* End Systems Calls */
