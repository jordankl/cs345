#ifndef KERNEL_H
#define KERNEL_H

#include <setjmp.h>
#include "type_defs.h"
#include "system_calls.h"
#include "semaphores.h"
#include "signals.h"

// ***********************************************************************
//
#define STARTUP_MSG	"CS345 WINTER 2011 YEAH!\n"
#define SHUTDOWN_MSG "Happy Happy Joy Joy"

// Utility directives
#define INTEGER(s)	((s)?(isdigit(*s)||(*s=='-')?(int)strtol(s,0,0):0):0)

#define MAX_ARGS			50
#define STACK_SIZE			(64*1024/sizeof(int))
#define MAX_CYCLES			CLOCKS_PER_SEC/2
#define INBUF_SIZE			256
#define TEN_SEC				10

// Default priorities
#define LOW_PRIORITY		1
#define MED_PRIORITY		5
#define HIGH_PRIORITY		10
#define VERY_HIGH_PRIORITY	20
#define HIGHEST_PRIORITY	99

#define MAX_TASKS 			127

// Task ID's
#define NO_TASK			-1
#define SHELL_TID		0
#define ALL_TID			-1
#define NO_PARENT		-(MAX_TASKS+1)

// Task state equates
#define S_READY	 	    1
#define S_RUNNING		2
#define S_STOPPED		3
#define S_BLOCKED		4
#define S_EXIT			5
#define S_ZOMBIE		6

#define KERNEL_RETURN_FROM_EXIT		MAXSYSCALLS+1
#define KERNEL_RETURN_FROM_SWAP		KERNEL_RETURN_FROM_EXIT+1
#define KERNEL_CHECK_SIGNALS		KERNEL_RETURN_FROM_SWAP+1

// task control block
typedef struct							// task control block
{
	char* name;							// task name
	int (*task)(int,char**);		    // task address
	int state;							// task state
	int priority;						// task priority (project 2)
	int argc;							// task argument count (project 1)
	char** argv;						// task argument pointers (project 1)
	int signal;							// task signals (project 1)
	sigmask_t signalMask;               // Mask defining blocked signals
	int handlingSignal;                 // task in/out of signal handler
	void (*sigIntHandler)(int);	        // task KSIGINT handler
	void (*sigKillHandler)(int);	    // task KSIGKILL handler
	void (*sigChldHandler)(int);	    // task KSIGCHLD handler
	void (*sigContHandler)(int);	    // task KSIGCONT handler
	void (*sigStopHandler)(int);	    // task KSIGSTOP handler
	void (*sigTstpHandler)(int);	    // task KSIGTSTP handler
	tid_t parent;						// task parent
	tid_t tgid;                         // task group id
	int result;                         // task return value on completion
	int RPT;							// task root page table (project 5)
	int rptIndex;						// task root page table index
	int cdir;							// task directory (project 6)
	Semaphore *event;					// blocked task semaphore
	void* stack;						// task stack
	jmp_buf context;					// task context pointer
	jmp_buf kill_context;				// task kill context
	trap_struct* system_call_params;    // connection to trap interface
} TCB;

// Task specific variables
#define CDIR		tcb[curTask].cdir
#define TASK_RPT	tcb[curTask].RPT

#ifdef GCCW32
// FOR LCC AND COMPATIBLE COMPILERS
#include <conio.h>
#define INIT_OS
#define GET_CHAR		(kbhit()?getch():0)
#define SET_STACK(s)	__asm("movl _temp,%esp");
#define RESTORE_OS
#define LITTLE	1
#define CLEAR_SCREEN	system("cls");
#endif

#if defined (GCC64) || defined (GCCOSX64)
// FOR GCC AND COMPATIBLE COMPILERS (Intel based MacBooks and Linux)
#include <fcntl.h>
#define INIT_OS		system("stty -echo -icanon");fcntl(1,F_SETFL,O_NONBLOCK);
#define GET_CHAR		getchar()
// There seems to be no small amount of confusion on the correct
// format for the extended asm command in gcc.  I am leaving
// all the copies in the code.  If you get an undefined symbol
// error, then you may want to look at one of these other
// variations. The current setting works in Mac OS X Tiger
// and Linux in the CS open labs (gcc 4.0.1 and gcc 4.1.2)
//
// #define SET_STACK __asm__ __volatile__("movl %0,%%esp"::"r"(temp):%esp);
// #define SET_STACK(s)	asm("movl temp,%esp")
#define SET_STACK(s)  asm("movq %0,%%rsp"::"r"(temp):"%rsp");

// enable canonical mode and echo
#define RESTORE_OS	system("stty icanon echo");
#define LITTLE	1
#define CLEAR_SCREEN	system("clear");
#endif

#if defined (GCCX86) || defined (GCCOSXX86)
// FOR GCC AND COMPATIBLE COMPILERS (Intel based MacBooks and Linux)
#include <fcntl.h>
#define INIT_OS		system("stty -echo -icanon");fcntl(1,F_SETFL,O_NONBLOCK);
#define GET_CHAR		getchar()
// There seems to be no small amount of confusion on the correct
// format for the extended asm command in gcc.  I am leaving
// all the copies in the code.  If you get an undefined symbol
// error, then you may want to look at one of these other
// variations. The current setting works in Mac OS X Tiger
// and Linux in the CS open labs (gcc 4.0.1 and gcc 4.1.2)
//
// #define SET_STACK __asm__ __volatile__("movl %0,%%esp"::"r"(temp):%esp);
// #define SET_STACK(s)	asm("movl temp,%esp")
#define SET_STACK(s)  asm("movl %0,%%esp"::"r"(temp):"%esp");

// enable canonical mode and echo
#define RESTORE_OS	system("stty icanon echo");
#define LITTLE	1
#define CLEAR_SCREEN	system("clear");
#endif

#ifdef NET
// FOR .NET AND COMPATIBLE COMPILERS
#include <conio.h>
#define INIT_OS
#define GET_CHAR		(_kbhit()?_getch():0)
#define SET_STACK(s) __asm mov ESP,s;
#define RESTORE_OS
#define LITTLE	1
#define CLEAR_SCREEN	system("cls");
#endif

#define SWAP_BYTES(v) 1?v:((((v)>>8)&0x00ff))|((v)<<8)
#define SWAP_WORDS(v) LITTLE?v:((SWAP_BYTES(v)<<16))|(SWAP_BYTES((v)>>16))

TCB* getTCB();

// Kernel Functions
tid_t _gettid();
void _setTrapParamsContext(trap_struct* system_call_params);
trap_struct* _getTrapParamsContext();
int _createTask(NewTask*);
void initTask();
void _swapTask(void);
int _killTask(int);
int _addToTics1SecList(Semaphore* s);
int _removeFromTics1SecList(Semaphore* s);
void _listTasks();
Semaphore* _helloWorld(int num, char* phrase, Semaphore* sem);

// ***********************************************************************
// system prototypes
void setNoTask(int);
int zombieTask(int taskId);
int dispatcher();
void initOS(void);

void pollInterrupts(void);
void powerDown(int code);

// ***********************************************************************
#define POWER_UP			   0

// The POWER_DOWN_ERROR is to catch error conditions in the simple OS
// that are unrecoverable.  Again, code must set context back to the
// 'main' function and let 'main' handle the 'powerDown' sequence.
#define POWER_DOWN_ERROR       1

// POWER_DOWN_QUIT is used to indicate that the shell has made a long
// jump into the main context of kernel.c with the intent to 'quit' the
// simple OS and power down.  The main function in kernel.c will catch
// the 'quit' request and 'powerDown'.
#define POWER_DOWN_QUIT       -2

// The POWER_DOWN_RESTART indicates the user issued the 'restart'
// command in the CLI.  Again, the 'main' function issues the
// 'powerDown' sequence.
#define POWER_DOWN_RESTART    -1

#endif // __os345_h__
