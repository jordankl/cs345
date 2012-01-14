#ifndef SHELL_H
#define SHELL_H

#include "type_defs.h"
#include "jurassic_park.h"
#include "commands.h"

#define TIC '\''

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-w
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

#define MAXCMDLINE	1024
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

struct job_t {              /* The job struct */
	tid_t tid;              /* job TID */
	int jid;                /* job ID [1, 2, ...] */
	int state;              /* UNDEF, BG, FG, or ST */
	char cmdline[MAXCMDLINE];  /* command line */
};

extern struct job_t jobs[MAXJOBS]; /* The job list */

/* Function prototypes */
/* Here are the functions that you will implement */
int shellTask(int argc, char* argv[]);
void eval(char *cmdline);
int parseline(const char *cmdline, char **argv); 
int get_argc_value(char** argv);
void do_bgfg(char **argv);
void waitfg(tid_t tid);
int find_cmd(Command cmd_list[], int num_cmds, char* cmd);
int builtin_cmd(char **argv);
/* Built-In Commands */
int cmd_quit(int argc, char* argv[]);
int cmd_sigKill(int argc, char* argv[]);
int cmd_help(int argc, char* argv[]);
int cmd_listjobs(int argc, char* argv[]);
int cmd_do_bgfg(int argc, char* argv[]);
int cmd_listTasks(int argc, char* argv[]);
int cmd_listSems(int argc, char* argv[]);
int cmd_reset(int argc, char* argv[]);
/* Signal Handlers */
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, tid_t tid, int state, char *cmdline);
int deletejob(struct job_t *jobs, tid_t tid); 
tid_t fgtid(struct job_t *jobs);
struct job_t *getjobtid(struct job_t *jobs, tid_t tid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int tid2jid(tid_t tid); 
void listjobs(struct job_t *jobs);
void sigquit_handler(int sig);

int kernel_error(char *msg);
//typedef void handler_t(int);
//handler_t *Signal(int signum, handler_t *handler);

#endif
