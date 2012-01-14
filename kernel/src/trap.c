#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>
#include "trap.h"
#include "kernel.h"

extern trap_struct* system_call_params;
extern int superMode;
extern jmp_buf k_context;

//@DISABLE_SWAPS

/**
 * trap: trap into the kernel
 * @formals: a pointer to the trap_struct on the user stack
 * @call_num: the system call number
 *
 * This function is called from the system call wrapper.  It should set
 * the context into the trap_struct, assign the parameters to the global
 * variable, and longjmp into the kernel.  If it receives a signal handler
 * it should be called before returning to the user program.
 */
void trap(trap_struct* formals, int call_num)
{
	int context_value;
	
	// Make sure you are not calling trap from kernel mode
	assert("Error: Trying to call trap from kernel mode\n" && !superMode);
	
	// Save Context so kernel can return to it
	context_value = setjmp(*((jmp_buf*)formals->params[0]));
	if (context_value == TRAP_NORMAL) {
		return;
	} else if (context_value > 0) {
		// Call the signal handler
		system_call_params->signal_handler(context_value);
		
		// longjmp back to kernel
		longjmp(k_context, KERNEL_CHECK_SIGNALS);
	} else {		
		// Assign values to globals
		system_call_params = formals;
		
		// longjmp to kernel context and return system call value
		longjmp(k_context, call_num);
	}
	
	// Never Reaches here
	return;
	
}

//@ENABLE_SWAPS

