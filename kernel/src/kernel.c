// kernel.c - OS Kernel
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the BYU CS345 projects.      **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

//@DISABLE_SWAPS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "kernel.h"
#include "shell.h"
#include "system_calls.h"
#include "trap.h"
#include "system_calls_kernel.h"
#include "signals.h"
#include "delta_clock.h"
#include "virtual_memory.h"
#include "messages.h"
#include "scheduler.h"
#include "lc3_simulator.h"
#include "fat.h"

// Probably want to remove some time
extern Message messages[NUM_MESSAGES];	    // process message buffers

// System Call Global Variables
trap_struct* system_call_params;

// **********************************************************************
// **********************************************************************
// global semaphores
int keyboardSemId;				// keyboard semaphore
int charReadySemId;				// character has been entered
int inBufferReadySemId;			// input buffer ready semaphore

// **********************************************************************
// **********************************************************************
// global system variables

static TCB tcb[MAX_TASKS];					// task control block
int taskSemIds[MAX_TASKS];	    // task semaphore
jmp_buf k_context;					// context of kernel stack
jmp_buf reset_context;				// context of kernel stack
jmp_buf new_task_context;
void* temp;							// temp pointer used in dispatcher
int kernel_started;

int superMode;						// system mode
static int curTask;						// current task #
long swapCount;						// number of re-schedule cycles
char inChar;						// last entered character
int charFlag;						// 0 => buffered input
int inBufPtr;						// input pointer into input buffer
char inBuffer[INBUF_SIZE];			// character input buffer

int pollClock;						// current clock()
int lastPollClock;					// last pollClock


// +++++ egm 22 Jan 2009
// Generates a unique task group ID when needed by createTask.
// Initialized in initOS.
int tgidCounter;
// -----

// Always add new system calls here
int (*sys_call_table[MAXSYSCALLS]) () = {	U_gettid,
											U_swapTask,
											U_semSignal,
											U_semWait,
											U_listTasks,
											U_helloWorld,
											U_killTask,
											U_createTask,
											U_sigKill,
											U_sigAction,
											U_sigProcMask,
											U_createSemaphore,
											U_deleteSemaphore,
											U_semTryLock,
											U_insertDeltaClock,
											U_deleteClockEvent,
											U_waittid,
											U_listSems,
											U_listDeltaClock,
											U_wait};

// **********************************************************************
// **********************************************************************
// OS startup
//
// 1. Init OS
// 2. Define reset longjmp vector
// 3. Define global system semaphores
// 4. Create CLI task
// 5. Enter scheduling/idle loop
//
int main(int argc, char* argv[])
{
	// All the 'powerDown' invocations must occur in the 'main'
	// context in order to facilitate 'killTask'.  'killTask' must
	// free any stack memory associated with current known tasks.  As
	// such, the stack context must be one not associated with a task.
	// The proper method is to longjmp to the 'reset_context' that
	// restores the stack for 'main' and then invoke the 'powerDown'
	// sequence.

	// Save context for restart (a system reset would return here...)
	int resetCode = setjmp(reset_context);
	superMode = TRUE;							// supervisor mode

	switch (resetCode)
	{
		case POWER_DOWN_QUIT:				// -2 = quit
			powerDown(0);
			printf("%s\n", SHUTDOWN_MSG);
			return 0;

		case POWER_DOWN_RESTART:			// -1 = restart
			powerDown(resetCode);
			printf("Restarting system...\n");

		case POWER_UP:							// 0 = startup
			break;

		default:
			printf("Shutting down due to error %d\n", resetCode);
			powerDown(resetCode);
			return 0;
	}

	// Output header message
	printf("%s", STARTUP_MSG);

	// Initalize OS
	initOS();

	// Schedule CLI task
	NewTask myShellTask;
	myShellTask.name = "myShell";
	myShellTask.task = shellTask;
	myShellTask.priority = MED_PRIORITY;
	myShellTask.argc = argc;
	myShellTask.argv = argv;
	myShellTask.parentHandlers = FALSE;
	myShellTask.tgidNew = TRUE;
	// Set context
	//if (setjmp(k_context) == 0) {
		_createTask(&myShellTask);
	//}

	// HERE WE GO................

	// Scheduling loop
	// 1. Check for asynchronous events (character inputs, timers, etc.)
	// 2. Choose a ready task to schedule
	// 3. Dispatch task
	// 4. Loop (forever!)

	// Update that the kernel has started
	kernel_started = TRUE;

	while(1)									// scheduling loop
	{
		// check for character / timer interrupts
		pollInterrupts();

		// schedule highest priority ready task
		if ((curTask = scheduler()) < 0) continue;

		// dispatch curTask, quit OS if negative return
		if (dispatcher() < 0) break;
	}											// end of scheduling loop

	// Exit os
	longjmp(reset_context, POWER_DOWN_QUIT);
	return 0;
}

/**
 * pollInterrupts: simulate asynchronous interrupts during idle loop
 *
 * This function checks for character inputs and ticks the delta clock.
 */
void pollInterrupts()
{
	// Watchdog Timer
	pollClock = clock();
	//assert("Timeout" && ((pollClock - lastPollClock) < MAX_CYCLES));
	lastPollClock = pollClock;

	// get character input
	inChar = GET_CHAR;
	if (inChar > 0)			// check for keyboard hit
	{
		_semSignal(charReadySemId);					// SIGNAL(charReady) (No Swap)

		if (charFlag == 0)
		{
			switch (inChar)
			{
				case '\r':
				case '\n':
				{
					inBuffer[inBufPtr++] = '\n';	// Copy return into inBuffer
					inBuffer[inBufPtr] = 0;         // Set the NULL terminator
					inBufPtr = 0;					// EOL, signal line ready
					printf("\n");					// echo character
					_semSignal(inBufferReadySemId);		// SIGNAL(inBufferReady)
					break;
				}

				case 0x17:							// ^w
				{
					if (KDEBUG) printf("Pressed ctrl-w\n");
					inBufPtr = 0;
					inBuffer[0] = 0;
					_sigKill(0, KSIGTSTP);			// send KSIGTSTP to task 0
					break;
				}

				case 0x18:							// ^x
				{
					if (KDEBUG) printf("Pressed ctrl-x\n");
					inBufPtr = 0;
					inBuffer[0] = 0;
					_sigKill(0, KSIGINT);			// send KSIGINT to task 0
					break;
				}

				// backspace (\b or ^h)
				case 0x08:
				case 0x7f:
				{
					if(inBufPtr > 0)
					{
						inBufPtr--;
						inBuffer[inBufPtr] = 0;
						printf("\b \b");
					}
					break;
				}

				default:
				{
					inBuffer[inBufPtr++] = inChar;
					inBuffer[inBufPtr] = 0;
					printf("%c", inChar);			// echo character
				}
			}
		}
		else
		{
			// single character mode
			inBufPtr = 0;
			inBuffer[inBufPtr] = 0;
		}
	}

	// Tick Delta Clock
	_tickDeltaClock();

	return;
}

/**
 * dispatcher: dispatch the current task
 * @return: 0 if successful, -1 if error
 *
 * The dispatcher will check to see if a task has received a KSIGKILL.
 * It will then switch on what state the task is in.  If it is ready, it
 * will be assigned to running.  If it is running, it will set the kernel
 * context, check for signals, and then execute.  If it is blocked, it
 * will break.  If stopped, it will check to see if it has received a
 * KSIGCONT.  If it has exited, it will signal the parent.
 */
int dispatcher()
{
	//printf("In dispatcher with task %d\n", curTask);

	// Variables for checking signals
	sigmask_t mask = 0;
	sigmask_t *p_mask = &mask;
	int kernelReturnValue; // Return value from kernel setjmp
	int isDefault = 0;

	checkAndreceiveKSigKill(curTask);

	// schedule task
	switch(tcb[curTask].state)
	{
		case S_READY:
		{
			tcb[curTask].state = S_RUNNING;	// set task to run
		}

		case S_RUNNING:
		{	// save kernel context for task SWAP's
			kernelReturnValue = setjmp(k_context);

			if ((kernelReturnValue == KERNEL_RETURN_FROM_EXIT) ||
				(kernelReturnValue == KERNEL_RETURN_FROM_SWAP)) {
				superMode = TRUE;
				break;
			}
			else if ((kernelReturnValue == KERNEL_CHECK_SIGNALS) ||
					 (kernelReturnValue == 0)) {
				superMode = TRUE;

				// Unblock a signal that was being handled
				tcb[curTask].handlingSignal = FALSE;
				_sigProcMask(KSIG_UNBLOCK, p_mask);

				// Check to see if it has a signal and is not handling
				// a signal already

				if (tcb[curTask].signal & ~(tcb[curTask].signalMask)) {


					// Check for KSIGKILL
					checkAndreceiveKSigKill(curTask);

					// Check for KSIGINT
					isDefault =
						(tcb[curTask].sigIntHandler == defaultKSigIntHandler);
					checkAndReceiveSignal(KSIGINT, 
										  isDefault,
										  p_mask,
										  tcb[curTask].sigIntHandler);

					// Check for KSIGCHLD
					isDefault =
						(tcb[curTask].sigChldHandler == defaultKSigChldHandler);
					checkAndReceiveSignal(KSIGCHLD,
										  isDefault, p_mask,
										  tcb[curTask].sigChldHandler);

					// Check for KSIGCONT
					isDefault = 1;
					checkAndReceiveSignal(KSIGCONT,
										  isDefault, p_mask,
										  tcb[curTask].sigContHandler);

					// CHeck for KSIGSTOP
					isDefault = 1;
					checkAndReceiveSignal(KSIGSTOP,
										  isDefault, p_mask,
										  tcb[curTask].sigStopHandler);

					// Check for KSIGTSTP
					isDefault =
						(tcb[curTask].sigTstpHandler == defaultKSigTstpHandler);
					checkAndReceiveSignal(KSIGTSTP,
										  isDefault, p_mask,
										  tcb[curTask].sigTstpHandler);
				}

				// Check the state of the task
				if (tcb[curTask].state == S_RUNNING) {
					// Assign superMode
					superMode = FALSE;

					// longjmp to context
					longjmp(tcb[curTask].context,TRAP_NORMAL);
				} else {
					// Task is not running anymore, get a new task
					//reschedule(curTask);
					break;
				}
			}
			else if (kernelReturnValue > 0) {
				// Assign superMode
				superMode = TRUE;

				// If I'm handling a signal, don't overwrite context
				if (tcb[curTask].handlingSignal == FALSE) {
					// Save the context to the current task
					memcpy(&(tcb[curTask].context),system_call_params->params[0], sizeof(jmp_buf));
				}

				// Run System Call
				if ((sys_call_table[kernelReturnValue-1])()) {
					// Check to see if I'm already handling a signal
					if (tcb[curTask].handlingSignal == TRUE) {
						// Assign superMode
						superMode = FALSE;

						longjmp(*((jmp_buf*)system_call_params->params[0]),TRAP_NORMAL);
					} else {
						// Check for signals
						longjmp(k_context,KERNEL_CHECK_SIGNALS);
					}
				}

				// Do not return from system call
				break;
			}
		}

	    case S_STOPPED:
		{
			// Check to see if the task has received a KSIGCONT
			// Maybe, in _sigKill, we can check to see if a task is
			// stopped and then change its state to S_RUNNING.  The signal
			// will be handled above.
			// This could help us move it to S_BLOCKED on a null event
			isDefault = 1;
			checkAndReceiveSignal(KSIGCONT,
								  isDefault, p_mask,
								  tcb[curTask].sigContHandler);
		    break;
		}

		case S_BLOCKED:
		{
			// A task is unblocked in _semSignal
			break;
		}

		case S_EXIT:
		{
			// If CLI, then quit scheduler
			if (curTask == 0) return -1;

			// Zombie current task
			zombieTask(curTask);

			break;
		}

		case S_ZOMBIE:
		{
			break;
		}

		default:
		{
			printf("Unknown Task[%d] State: %d\n", curTask, tcb[curTask].state);
			longjmp(reset_context, POWER_DOWN_ERROR);
		}
	}

	return 0;
}

/**
 * _swapTask: do a context switch to next task
 *
 * This function should change the state of the current task to S_READY
 * reschedule the task, and longjmp to the kernel.
 */
void _swapTask()
{
	assert("Trying to call swapTask in User Mode" && superMode);

	// Swapping a task while handling a signal will corrupt task context
	if (tcb[curTask].handlingSignal) {
		return;
	}

	// Increment swap cycle counter
	swapCount++;

	// Move task state to ready and reschedule
	if (tcb[curTask].state == S_RUNNING) {
		tcb[curTask].state = S_READY;
		reschedule(curTask);
	} else {
		assert("Unexpected state in swapTask" && tcb[curTask].state == S_BLOCKED);
	}

	// Jump to kernel mode
	longjmp(k_context, KERNEL_RETURN_FROM_SWAP);

}

TCB* getTCB()
{
	//assert("Trying to call getTCB in user mode" && superMode);

	return tcb;
}

// **********************************************************************
// **********************************************************************
// system utility functions
// **********************************************************************
// **********************************************************************

void setNoTask(int taskId)
{
	tcb[taskId].name = NULL;
	tcb[taskId].task = NULL;
	tcb[taskId].state = -1;
	tcb[taskId].priority = -1;
	tcb[taskId].argc = -1;
	tcb[taskId].argv = NULL;
	tcb[taskId].signal = 0;
	tcb[taskId].signalMask = 0;
	tcb[taskId].handlingSignal = FALSE;
	tcb[taskId].sigIntHandler = NULL;
	tcb[taskId].sigKillHandler = NULL;
	tcb[taskId].sigChldHandler = NULL;
	tcb[taskId].sigContHandler = NULL;
	tcb[taskId].sigStopHandler = NULL;
	tcb[taskId].sigTstpHandler = NULL;
	tcb[taskId].parent = NO_PARENT;
	tcb[taskId].tgid = NO_PARENT;
	tcb[taskId].result = 0;
	tcb[taskId].RPT = 0;
	tcb[taskId].cdir = 0;
	tcb[taskId].event = NULL;
	tcb[taskId].stack = NULL;
	//jmp_buf context;
	//jmp_buf kill_context;
	tcb[taskId].system_call_params = NULL;

	// Clear any pending signal for reuse by other tasks
	if (taskSemIds[taskId] != NULL_SEMAPHORE) {
		_semTryLock(taskSemIds[taskId]);
	}
}


// **********************************************************************
// **********************************************************************
// create global semaphores
void createGlobalSemaphores() {
	int i;
	char buf[8];

	// create task semaphore
	for (i=0; i<MAX_TASKS; i++) {
		sprintf(buf, "task%d", i);
		taskSemIds[i] = _createSemaphore(buf, BINARY, 0);

		// Critical: assigning NO_PARENT prevents
		// killTask from ever deleting the task
		// semaphores because they are not owned
		// by anyone.

		_getSemaphore(taskSemIds[i])->taskNum = NO_PARENT;
	}

	// create other global/system semaphores here
	//?? vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	charReadySemId = _createSemaphore("charReady", BINARY, 0);
	inBufferReadySemId = _createSemaphore("inBufferReady", BINARY, 0);
	keyboardSemId = _createSemaphore("keyboard", BINARY, 1);

	//?? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

// **********************************************************************
// **********************************************************************
// null global semaphores
void clearGlobalSemaphoreIDs() {
	int i;

	// Null task semaphores
	for (i=0; i<MAX_TASKS; i++) {
		taskSemIds[i] = NULL_SEMAPHORE;
	}

	// Null other global/system semaphores here
	charReadySemId = NULL_SEMAPHORE;
	inBufferReadySemId = NULL_SEMAPHORE;
	keyboardSemId = NULL_SEMAPHORE;
}

// **********************************************************************
// **********************************************************************
// initialize operating system
void initOS()
{
	int i;

	// make any system adjustments (for unblocking keyboard inputs)
	INIT_OS

	// reset system variables
	curTask = NO_TASK;						// current task #
	kernel_started = 0;
	swapCount = 0;						// number of scheduler cycles
	inChar = 0;							// last entered character
	charFlag = 0;						// 0 => buffered input
	inBufPtr = 0;						// input pointer into input buffer
	tgidCounter = 0;

	// capture current time
	lastPollClock = clock();		// last pollClock

	// init system tcb's
	for (i=0; i<MAX_TASKS; i++) {
		taskSemIds[i] = NULL_SEMAPHORE;
		setNoTask(i);
	}

	// initalize message buffers
	for (i=0; i<NUM_MESSAGES; i++) {
		messages[i].to = -1;
	}

	// initialize lc-3 memory
	initLC3Memory(LC3_MEM_FRAME, 0xF000>>6);
	initVM();

	// Critical: must initialize the scheduler _before_
	// creating the Global Semaphores!
	initScheduler();

	// ?? initialize all execution queues
	createGlobalSemaphores();

	return;
}



// **********************************************************************
// **********************************************************************
// Causes the system to shut down. Use this for critical errors
void powerDown(int code)
{
	int i;
	printf("PowerDown Code %d\n", code);

	// release all system resources.
	printf("Recovering Task Resources...\n");

	// kill all tasks
	for (i = MAX_TASKS-1; i >= 0; i--) {
		if(tcb[i].name) {
			_killTask(i);
		}
	}

	_deleteAllSemaphores();
	clearGlobalSemaphoreIDs();

	// free ready queue and queue memory
	clearScheduler();

	// ?? release any other system resources
	// ?? deltaclock (project 3)
	system_call_params = NULL;

    RESTORE_OS
	return;
}

// **********************************************************************
// **********************************************************************
// create task
int _createTask(NewTask* taskInfo)
{
	int tid;
	int i = 0, len = 0;
	int result = 0;

	assert("Error: Trying to call _createTask in User Mode\n" && superMode);

	// find an open tcb entry slot
	for (tid=0; tid<MAX_TASKS; tid++) {
		if (tcb[tid].name == 0) {
			// copy task name
			tcb[tid].name = (char*)malloc(strlen(taskInfo->name)+1);
			strcpy(tcb[tid].name, taskInfo->name);

			// set task address and other parameters
			tcb[tid].task = taskInfo->task;			// task address
			tcb[tid].state = S_RUNNING;				// Running task state
			tcb[tid].priority = taskInfo->priority;	// task priority
			tcb[tid].parent = curTask;				// parent
			tcb[tid].argc = taskInfo->argc;			// argument count

			// ?? malloc new argv parameters
			tcb[tid].argv = (char **)malloc(taskInfo->argc * sizeof(char *));
			for (i = 0 ; i < taskInfo->argc ; i++) {
				assert(taskInfo->argv[i]);
				len = strlen(taskInfo->argv[i]) + 1;
				tcb[tid].argv[i] = (char*)malloc(len * sizeof(char));
				assert(tcb[tid].argv[i]);
				strcpy(tcb[tid].argv[i], taskInfo->argv[i]);
			}

			//tcb[tid].argv = argv;			// argument pointers

			tcb[tid].event = 0;				// suspend semaphore

			tcb[tid].RPT = 0;

			// Allocate a root page table to LC-3 tasks and the shell
			if (tid == 0 || taskInfo->task == lc3Task) {
				tcb[tid].RPT = getFreeRPT();
			}

			if (curTask != -1) {
				tcb[tid].cdir = tcb[curTask].cdir;			// inherit parent cDir (project 6)
			} else {
				tcb[tid].cdir = 0;
			}
			// signals
			tcb[tid].signal = 0;
			tcb[tid].handlingSignal = FALSE;
			tcb[tid].signalMask = 0;

			tcb[tid].sigIntHandler = defaultKSigIntHandler;
			tcb[tid].sigKillHandler = KSigKillHandler;
			tcb[tid].sigChldHandler = defaultKSigChldHandler;
			tcb[tid].sigContHandler = KSigContHandler;
			tcb[tid].sigStopHandler = KSigStopHandler;
			tcb[tid].sigTstpHandler = defaultKSigTstpHandler;

			if (tid && taskInfo->parentHandlers) {
				// inherit parent signal handlers that can be
				// customized
				tcb[tid].sigIntHandler = tcb[curTask].sigIntHandler;
				tcb[tid].sigChldHandler = tcb[curTask].sigChldHandler;
				tcb[tid].sigTstpHandler = tcb[curTask].sigTstpHandler;
				tcb[tid].signalMask = tcb[curTask].signalMask;
			}

			if (taskInfo->tgidNew) {
				tcb[tid].tgid = ++tgidCounter;
			} else {
				tcb[tid].tgid = tcb[curTask].tgid;
			}

			// We are going to use result to track the child status for reaping
			// including the return value on normal exit.
			tcb[tid].result = 0;

			// Each task must have its own stack and stack pointer.
			tcb[tid].stack = malloc(STACK_SIZE * sizeof(int));

			// Assign the tid to the trap_struct
			if (system_call_params != NULL && kernel_started == TRUE) {
				system_call_params->return_value.return_tid = tid;
			}

			// Reschedule current task
			if (curTask != NO_TASK) {
				reschedule(curTask);
			}

			// Change the curTask to reflect the new task
			curTask = tid;

			if (kernel_started == FALSE) {
				result = setjmp(k_context);
			}

			if (result == 0) {
				// Move to new task stack (leave room for return value/address)
				temp = (int*)tcb[curTask].stack + (STACK_SIZE-8);

				// Set up the stack
				SET_STACK(temp)

				// Call initTask();
				initTask();
			}

			return 0;
		}
	}

	// Assign the tid to the trap_struct
	if (system_call_params != 0) {
		system_call_params->return_value.return_tid = NO_TASK;
	}

	// tcb full!
	return -1;
}

void initTask()
{
	int result;

	// Check to see if the kernel has started
	if (kernel_started == FALSE) {
		// Save the context
		if (setjmp(tcb[curTask].context) == 0) {
			// Reschedule
			reschedule(curTask);

			// Return to _createTask
			longjmp(k_context,1);
		}
	}

	// Set up the kill context
	if (setjmp(tcb[curTask].kill_context)) {
		// Value is set in tcb[curTask].result
	} else {
		// Change to user mode
		superMode = FALSE;

		// Swap
		swapTask();

		// Call function
		result = (*tcb[curTask].task)(tcb[curTask].argc, tcb[curTask].argv);

		// Change to kernel mode
		superMode = TRUE;

		// I am only going to cosider the bottom 8-bits of result,
		// and I am going to shift it up 24-bits so it does not
		// interfere with the KSIG* bits used for the WIFK* functions
		tcb[curTask].result = (result & 0x000000ff) << 24;
	}

	// Print finished statement
	if (KDEBUG) printf("Task[%d] returned %d\n", curTask, result);

	// Set task to exit state
	tcb[curTask].state = S_EXIT;

	// Put the task back into the ready queue
	reschedule(curTask);

	// return to kernel mode
	longjmp(k_context, KERNEL_RETURN_FROM_EXIT);
}

// **********************************************************************
// **********************************************************************
// zombie task
//
int zombieTask(int taskId)
{
	int i = 0;
	int ptid = 0;
	Semaphore *s = NULL;

	// assert that you are not pulling the rug out from under yourself!
	assert("zombieTask Error" && tcb[taskId].name && superMode);

	// ++++ egm 22-Jan-2009
	// Moved all reporting behind the KDEBUG flag.  Let shell
	// or task creater report status etc.
	if (KDEBUG) printf("Zombie Task %s\n", tcb[taskId].name);
	// -----

	// ?? delete task from system queues

	s = tcb[taskId].event;
	if (s != NULL) {
		assert(removeNode(s->blockedList, taskId));
		tcb[taskId].event = NULL;
	}

	// Signal parent and leave tcb[curTask].name for reaping
	ptid = tcb[taskId].parent;
	tcb[taskId].state = S_ZOMBIE;

	// Orphan offspring (sorry)
	for (i = 0 ; i < MAX_TASKS ; ++i ) {
		if (tcb[i].name && tcb[i].parent == taskId) {
			tcb[i].parent = NO_PARENT;
		}
	}

	// Reap any orphaned jobs if pertinent (kernel house cleaning)
	// Danger: killTask can be called in other ways, so assuming
	// curTask is dangerouse.  Create internal waittid to take a
	// taskId so it can be called in this way.
	while (_wait(taskId, NULL) > 0);
	//while (_wait(NULL) > -1);

	// Signal parent process
	if (ptid != NO_PARENT) {
		_semSignal(taskSemIds[ptid]);
		_sigKill(ptid, KSIGCHLD);
	}

	// Signal self to indicate a need to be reaped
	//
	// WARNING: we are going to use the semaphore for reaping
	// so we need to be careful with others who are looking
	// at this semaphore.  Signaling the semaphore means the
	// task needs to be reaped.  We follow the same
	// protocol when a task stops.
	_semSignal(taskSemIds[taskId]);

	return 0;
}

// **********************************************************************
// **********************************************************************
// kill task
//
int _killTask(int taskId)
{
	int i = 0;

    assert("Error: Trying to call _killTask in User Mode\n" && superMode);

	// Make sure you are fit for killing
	assert("killTask Error" && tcb[taskId].name);

	// Delete all the sempahores for the task
	// Actually dont, by request of Dr. Mercer
	//_deleteAllSemaphoresForTask(taskId);

	// Remove the task from scheduler
	deschedule(taskId);

	// Free up the stack for the task
	free(tcb[taskId].stack);
	tcb[taskId].stack = NULL;

	// Free up the name of the task
	free(tcb[taskId].name);
	tcb[taskId].name = NULL;

	// ?? delete argv array as appropriate
	assert(tcb[taskId].argv);;
	for ( i = 0 ; i < tcb[taskId].argc ; ++i ) {
		assert(tcb[taskId].argv[i]);
		free(tcb[taskId].argv[i]);
	}
	free(tcb[taskId].argv);

	// ?? delete task from system queues

	setNoTask(taskId);

	return 0;
}

/**
 * _gettid(): return the task id of the currently running task
 * @return: the task if of the currently running task
 *
 * This system call returns the task if of the currently running task.
 * A user task can call this system call using gettid().
 */
tid_t _gettid()
{
    assert("Error: Trying to call _gettid in User Mode\n" && superMode);
	return curTask;
}

/**
 * _setTrapParamsContext(): saves the system_call_params in the tcb for the curTask
 * @return: void
 *
 * This system call saves the system_call_params into the tcb of the 
 * current running task.
 */
void _setTrapParamsContext(trap_struct* system_call_params) {
	assert("Error: Trying to call _setTrapParamsContext in User Mode\n" && superMode);
	tcb[curTask].system_call_params = system_call_params;
}

/**
 * _setTrapParamsContext(): returns system_call_params in the tcb of the curTask
 * @return: trap_struct*
 *
 * This system call returns the system_call_params in the tcb of the 
 * current running task.
 */
trap_struct* _getTrapParamsContext() {
	assert("Error: Trying to call _getTrapParamsContext in User Mode\n" && superMode);
	return tcb[curTask].system_call_params;
}



/**
 * _listTasks: print the state of all tasks
 *
 * This prints the state of all tasks.  If the task is blocked, it prints the
 * name of the semaphore on which it is blocked.
 */
void _listTasks()
{
	int i;

    assert("Error: Trying to call _listTasks in User Mode\n" && superMode);

	for (i=0; i<MAX_TASKS; i++) {
		if (tcb[i].name) {
			printf("\t%d/%-d %-15s %-2d ", i, tcb[i].parent,
				   tcb[i].name, tcb[i].priority);
			if (tcb[i].state == S_READY)	printf("Ready");
			else if (tcb[i].state == S_RUNNING) printf("Running");
			else if (tcb[i].state == S_STOPPED) printf("Stopped");
			else if (tcb[i].state == S_BLOCKED) printf("Blocked  %s",
													   tcb[i].event->name);
			else if (tcb[i].state == S_EXIT)	printf("Exiting");
			else if (tcb[i].state == S_ZOMBIE)	printf("Zombie");
			else								printf("Undefined");
			printf("\n");
		}
	}
}

/**
 * _helloWorld: used for old system calls lab
 */
Semaphore* _helloWorld(int num, char* phrase, Semaphore* sem)
{
	// Assert superMode
	assert("Trying to run helloWorld in user mode!\n" && superMode);

	// Do test
	printf("HELLO WORLD SYSTEM CALL with superMode: %d\n",superMode);
	printf("Received Number: %d\n", num);
	printf("Received Phrase: %s\n", phrase);
	printf("Received Semaphore: %s\n", sem->name);

	// Return passed semaphore
	return sem;

}
//@ENABLE_SWAPS
