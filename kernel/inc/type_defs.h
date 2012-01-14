#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

typedef int sigmask_t;                  // mask type for managing signals
#ifndef NET
typedef int bool;
#endif
typedef int tid_t;

typedef struct {
	char* name;					// task name
	int (*task)(int, char**);	// task address
	int priority;				// task priority
	int argc;					// task argument count
	char** argv;				// task argument pointers
	int parentHandlers;			// Inherit parent handlers?
	int tgidNew;				// Create a new task group ID?
} NewTask;

#define FALSE	0
#define TRUE	1
// Modify the DEFINEs in the Makefile to enable 
// KDEBUG: -DKDEBUG=TRUE or -DKDEBUG=FALSE
// #define KDEBUG	TRUE
#define EMPTY	-1

#define MAX_STRING_SIZE 127

#endif
