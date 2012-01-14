// shell.c - Command Line Processor
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "shell.h"
#include "type_defs.h"
#include "semaphores.h"
#include "system_calls.h"
#include "kernel.h"
#include "signals.h"


// +++++ egm
// The 'reset_context' comes from 'main' in kernel.c.  Proper shut-down
// procedure is to long jump to the 'reset_context' passing in the
// power down code from 'kernel.h' that indicates the desired behavior.

/* Global variables */
extern jmp_buf reset_context;
int sleepSemId;
int activejobs = 0;
char prompt[] = ">> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
char sbuf[MAXLINE];         /* for composing sprintf messages */
extern long swapCount;					// number of scheduler cycles
extern char inBuffer[];					// character input buffer
extern int inBufferReadySemId;		// input buffer ready semaphore
bool diskMounted = 0;					// disk has been mounted
extern char dirPath[];					// directory path
/* End global variables */

struct job_t jobs[MAXJOBS]; /* The job list */

// ***********************************************************************
// myShell - command line interpreter
//
// Project 1 - implement a Shell (CLI) that:
//
// 1. Prompts the user for a command line.
// 2. WAIT's until a user line has been entered.
// 3. Parses the global char array inBuffer.
// 4. Creates new argc, argv variables using malloc.
// 5. Searches a command list for valid OS commands.
// 6. If found, perform a function variable call passing argc/argv variables.
// 7. Supports background execution of non-intrinsic commands.
//
int shellTask(int argc, char* argv[])
{
	char cmdline[MAXLINE];
	char name[MAX_STRING_SIZE];
	int emit_prompt = 1; /* emit prompt (default) */
	int i = 0;

	sprintf(name, "P1_shellTask-sleep-%d", gettid());
	sleepSemId = createSemaphore(name, BINARY, 0);

	emit_prompt = 1;     /* Clear if you want to supress prompt */

	/* Install the signal handlers */
	sigAction(sigint_handler, KSIGINT);    /* ctrl-x */
	sigAction(sigtstp_handler, KSIGTSTP);  /* ctrl-w */
	sigAction(sigchld_handler, KSIGCHLD);  /* Terminated or stopped child */

	/* Initialize the job list */
	initjobs(jobs);

	while(1) {

		// output prompt
		if (diskMounted && emit_prompt) {
			printf("%s%s", dirPath, prompt);
			fflush(stdout);
		}
		else if (emit_prompt) {
			printf("%ld%s", swapCount, prompt);
			fflush(stdout);
		}

		semWait(inBufferReadySemId);			/* wait for input buffer semaphore */
		if (!inBuffer[0]) continue;		    /* ignore blank lines */
		swapTask();								/* do context switch */

		strncpy(cmdline, inBuffer, MAXLINE);
		for (i=0; i<INBUF_SIZE; i++) {
			inBuffer[i] = 0;                /* clear inBuffer */
		}

		/* Evaluate the command line */
		eval(cmdline);
		fflush(stdout);
	}

	// This code I believe is unreachable
	deleteClockEvent(sleepSemId);
	deleteSemaphore(sleepSemId);
	sleepSemId = NULL_SEMAPHORE;

	return 0;

} // end shellTask


/**
 * eval - Evaluate the command line that the user has just typed in
 * @cmdline: the command line entered into the shell
 *
 * If the user has requested a built-in command then execute it
 * immediately. Otherwise, fork a child process and run the job in the
 * context of the child. If the job is running in the foreground, wait
 * for it to terminate and then return.
 */

void eval(char *cmdline)
{

  char *argv[MAXARGS]; /* argv for execve() */
  int bg;              /* should the job run in bg or fg? */


  tid_t tid = 0;       /* task id */
  sigmask_t mask;      /* signal mask */


  /* Parse command line */
  bg = parseline(cmdline, argv);
  if (argv[0] == NULL)  {
    return;   /* ignore empty lines */
  }

  if (!builtin_cmd(argv)) {
	  
	  if (activejobs >= MAXJOBS) {
		  printf("ERROR: already at MAXJOBS of %d\n", MAXJOBS);
		  return;
	  }

	  // Create and start the task
	  sigEmptySet(&mask);

          if (!bg) {
                waitfg(tid); // wait for process to end if its in the foreground
          }
  }
  return;
}

/**
 * parseline: Parse the command line and build the argv array.
 * @cmdline: the command line entered
 * @argv: the array of parsed strings from the command line
 * @return: 1 if the command should run in the background, 0 otherwise
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv)
{
  static char array[MAXLINE]; /* holds local copy of command line */
  char *buf = array;          /* ptr that traverses command line */
  char *delim;                /* points to first space delimiter */
  int argc;                   /* number of args */
  int bg;                     /* background job? */

  strcpy(buf, cmdline);
  buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) { /* ignore leading spaces */
    buf++;
  }

  /* Build the argv list */
  argc = 0;
  if (*buf == TIC) {
    buf++;
    delim = strchr(buf, TIC);
  }
  else {
    delim = strchr(buf, ' ');
  }

  while (delim) {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) {/* ignore spaces */
      buf++;
	}

    if (*buf == TIC) {
      buf++;
      delim = strchr(buf, TIC);
    }
    else {
      delim = strchr(buf, ' ');
    }
  }
  argv[argc] = NULL;

  if (argc == 0) {  /* ignore blank line */
    return 1;
  }

  /* should the job run in the background? */
  if ((bg = (*argv[argc-1] == '&')) != 0) {
    argv[--argc] = NULL;
  }

  return bg;
}

/**
 * find_cmd: find a command
 * @cmd_list: the array of commands
 * @num_cmds: the number of commands in cmd_list
 * @cmd: the name of the command
 * @return: the index of the command, -1 if not found
 *
 * This function is used by the shell to find the index of the command
 * in the command list arrays.
 */

int find_cmd(Command cmd_list[], int num_cmds, char* cmd) {
	int i;

	assert(cmd != NULL);

	// look for command
	for (i = 0; i < num_cmds; i++)
	{
		if (!strcmp(cmd, cmd_list[i].command) ||
			!strcmp(cmd, cmd_list[i].shortcut))
		{
			return i;
		}
	}
	return -1;
}

/**
 * Return the number of arguments in an argv list.  Assumed null
 * termination.
 */

int get_argc_value(char** argv) {
	int argc = 0;
	while (argv[argc++]);
	return argc - 1;
}

/**
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv)
{
  /* $begin handout */
  char *cmd = argv[0];
  int index = 0;

  if (!strcmp(cmd, "&")) { /* Ignore singleton & */
    return 1;
  }

  index = find_cmd(builtin_cmd_list, number_builtin_cmds, cmd);
  if (index < 0 ) {
	  return 0;     /* not a builtin command */
  }

  (*builtin_cmd_list[index].func)(get_argc_value(argv), argv);

  return 1;
}

/**
 * waitfg - Block until task tid is no longer the foreground task
 */
void waitfg(tid_t tid)
{
  struct job_t *j = getjobtid(jobs, tid);

  /* The FG job has already completed and been reaped by the handler */
  if (!j) {
    return;
  }

  /* Wait for task tid to longer be the foreground task.
     Note: using pause() instead of sleep() would introduce a race
     that could cause us to miss the signal */
	while (j->tid == tid && j->state == FG) {
	  insertDeltaClock(10,sleepSemId,0);
	  semWait(sleepSemId);
  }

  if (verbose) {
    printf("waitfg: Task (%d) no longer the fg task\n", tid);
  }

  /* $end handout */
  return;
}

/**
 * cmd_quit
 */
int cmd_quit(int argc, char* argv[])
{
	// powerdown OS345
	longjmp(reset_context, POWER_DOWN_QUIT);
	return 0;
}

/**
 * cmd_listTasks
 */
int cmd_listTasks(int argc, char* argv[])
{
	// Do System Call
	listTasks();

	// Return
	return 0;
}

/**
 * cmd_listSems
 */
int cmd_listSems(int argc, char* argv[])
{
	// Do System Call
	listSems();

	// Return
	return 0;
}


/**
 * cmd_sigKill
 */
int cmd_sigKill(int argc, char* argv[])
{
	tid_t tid = 0;
	int signo = 0;

	/* Ignore command if no argument */
	if (argv[1] == NULL ||
		argv[2] == NULL ||
		!(argv[1][0] == '-' && isdigit(argv[1][1])) ||
		!(argv[2][0] == '-' || isdigit(argv[2][0]))) {
		printf("usage: sigKill -<signal> [-]<tid>\n");
		printf("\t -<tid> sends to all tasks in tid's group\n");
		printf("\tKSIGINT = %d\n", KSIGINT);
		printf("\tKSIGKILL = %d\n", KSIGKILL);
		printf("\tKSIGCONT= %d\n", KSIGCONT);
		printf("\tKSIGSTOP = %d\n", KSIGSTOP);
		printf("\tKSIGTSTP = %d\n", KSIGTSTP);
		return 1;
	}

	/* Parse the required signal number */
	signo = -(atoi(argv[1]));

	/* Parse the required TID */
	tid = atoi(argv[2]);

	if (KDEBUG) {
		printf( "sigKill: signo = %d, tid = %d\n", signo, tid);
	}

	sigKill(tid,signo);

	/* $end handout */
	return 0;
}

/**
 * cmd_reset
 */
int cmd_reset(int argc, char* argv[])
{
	longjmp(reset_context, POWER_DOWN_RESTART);
	// not necessary as longjmp doesn't return
	return 0;

}

/**
 * cmd_help
 */
int cmd_help(int argc, char* argv[])
{
	int i;

	printf("Built-In Commands:\n");
	for (i = 0; i < number_builtin_cmds; i++) {
		printf("\t%s (%s): %s\n",	builtin_cmd_list[i].command,
									builtin_cmd_list[i].shortcut,
									builtin_cmd_list[i].description);
	}

	printf("Other Commands:\n");
	for (i = 0; i < number_commands; i++) {
		printf("\t%s (%s): %s\n",	command_list[i].command,
									command_list[i].shortcut,
									command_list[i].description);
	}

	return 0;
}

/**
 * cmd_listjobs
 */
int cmd_listjobs(int argc, char* argv[])
{
	listjobs(jobs);
	return 0;
}

/**
 * cmd_do_bgfg
 */
int cmd_do_bgfg(int argc, char* argv[])
{
	/* $begin handout */
	struct job_t *jobp=NULL;
	sigmask_t mask;

	/* Ignore command if no argument */
	if (argv[1] == NULL) {
		printf("%s command requires TID or %%jobid argument\n", argv[0]);
		return 0;
	}

    if (sigEmptySet(&mask) < 0) {
        kernel_error("sigemptyset error");
    }
    if (sigAddSet(&mask, KSIGCHLD)) {
        kernel_error("sigaddset error");
    }
    if (sigAddSet(&mask, KSIGINT)) {
        kernel_error("sigaddset error");
    }
    if (sigAddSet(&mask, KSIGTSTP)) {
        kernel_error("sigaddset error");
    }
    if (sigProcMask(KSIG_BLOCK, &mask) < 0) {
        kernel_error("sigprocmask error");
    }

	/* Parse the required TID or %JID arg */
	if (isdigit(argv[1][0])) {
		tid_t tid = atoi(argv[1]);
		if (!(jobp = getjobtid(jobs, tid))) {
			printf("(%d): No such task\n", tid);
			sigProcMask(KSIG_UNBLOCK, &mask);
			return 0;
		}
	}
	else if (argv[1][0] == '%') {
		int jid = atoi(&argv[1][1]);
		if (!(jobp = getjobjid(jobs, jid))) {
			printf("%s: No such job\n", argv[1]);
			sigProcMask(KSIG_UNBLOCK, &mask);
			return 0;
		}
	}
	else {
		printf("%s: argument must be a TID or %%jobid\n", argv[0]);
		sigProcMask(KSIG_UNBLOCK, &mask);
		return 0;
	}

	/* bg command */
	if (!strcmp(argv[0], "bg")) {
		if (sigKill(-(jobp->tid), KSIGCONT) < 0) {
			//	kernel_error("kill (bg) error");
		}
		jobp->state = BG;
		printf("[%d] (%d) %s", jobp->jid, jobp->tid, jobp->cmdline);
		sigProcMask(KSIG_UNBLOCK, &mask);

	}

	/* fg command */
	else if (!strcmp(argv[0], "fg")) {
		if (sigKill(-(jobp->tid), KSIGCONT) < 0) {
			//	kernel_error("kill (fg) error");
		}
		jobp->state = FG;
		sigProcMask(KSIG_UNBLOCK, &mask);
		waitfg(jobp->tid);
	}
	else {
		printf("do_bgfg: Internal error\n");
		sigProcMask(KSIG_UNBLOCK, &mask);
		return 0;
	}

	/* $end handout */
	sigProcMask(KSIG_UNBLOCK, &mask);
	return 1;
}


/*****************
 * Signal handlers
 *****************/
//@DISABLE_SWAPS
/*
 * sigchld_handler - The kernel sends a KSIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a KSIGSTOP or KSIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig)
{
    return;
}

/*
 * sigint_handler - The kernel sends a KSIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
    return;
}

/*
 * sigtstp_handler - The kernel sends a KSIGTSTP to the shell whenever
 * the user types ctrl-z at the keyboard. Catch it and suspend the
 * foreground job by sending it a KSIGTSTP.
 */
void sigtstp_handler(int sig) {
    return;
}
//@ENABLE_SWAPS
/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
	job->tid = 0;
	job->jid = 0;
	job->state = UNDEF;
	job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
	int i;

	for (i = 0; i < MAXJOBS; i++)
		clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs)
{
	int i, max=0;

	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].jid > max)
			max = jobs[i].jid;
	return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, tid_t tid, int state, char *cmdline)
{
	int i;

	if (tid < 1)
		return 0;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].tid == 0) {
			jobs[i].tid = tid;
			jobs[i].state = state;
			jobs[i].jid = i+1;
			activejobs++;
			strcpy(jobs[i].cmdline, cmdline);
			if(KDEBUG) {
				printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].tid, jobs[i].cmdline);
			}
			return 1;
		}
	}
	printf("Tried to create too many jobs\n");
	return 0;
}

/* deletejob - Delete a job whose TID=tid from the job list */
int deletejob(struct job_t *jobs, tid_t tid)
{
	int i;

	if (tid < 1)
		return 0;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].tid == tid) {
			clearjob(&jobs[i]);
			activejobs--;
			return 1;
		}
	}
	return 0;
}

/* fgtid - Return TID of current foreground job, 0 if no such job */
tid_t fgtid(struct job_t *jobs) {
	int i;

	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].state == FG)
			return jobs[i].tid;
	return 0;
}

/* getjobtid  - Find a job (by TID) on the job list */
struct job_t *getjobtid(struct job_t *jobs, tid_t tid) {
	int i;

	if (tid < 1)
		return NULL;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].tid == tid)
			return &jobs[i];
	return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
	int i;

	if (jid < 1)
		return NULL;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].jid == jid)
			return &jobs[i];
	return NULL;
}

/* tid2jid - Map task ID to job ID */
int tid2jid(tid_t tid)
{
	int i;

	if (tid < 1)
		return 0;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].tid == tid) {
			return jobs[i].jid;
		}
	return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
	int i;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].tid != 0) {
			printf("[%d] (%d) ", jobs[i].jid, jobs[i].tid);
			switch (jobs[i].state) {
				case BG:
					printf("Running ");
					break;
				case FG:
					printf("Foreground ");
					break;
				case ST:
					printf("Stopped ");
					break;
				default:
					printf("listjobs: Internal error: job[%d].state=%d ",
						   i, jobs[i].state);
			}
			printf("%s", jobs[i].cmdline);
		}
	}
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/**
 * kernel_error: report and shutdown kernel.
 */
int kernel_error(char *msg)
{
  fprintf(stdout, "%s: panic abort\n", msg);
  longjmp(reset_context, POWER_DOWN_QUIT);
}

