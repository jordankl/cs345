#ifndef TRAP_H
#define TRAP_H

#include "type_defs.h"

#define TRAP_NORMAL		1

/**
 * struct return_union
 * @return_pointer
 * @return_tid
 * @return int
 */
typedef union {
	void*	return_pointer;
	tid_t	return_tid;
	int		return_int;
} return_union;

/**
 * struct trap_struct
 * @params: an array of void pointers on the user stack; 
 *			params[0] is a jmp_buf context
 * @return_value: the union of return values
 * @signal_handler: a pointer to the signal handler is it needs to be called
 */
typedef struct {
	void**	params;
	return_union return_value;
	void (*signal_handler)(int);
} trap_struct;

void trap(trap_struct*,int);

#endif
