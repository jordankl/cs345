## -------------------------------------------------------------------
##
## Author:              Eric Mercer (2010)
##
## -------------------------------------------------------------------

## nmake -f studio.mak <target>

## Specific defines for the build process
## directives
#ARCH = GCCW32
ARCH = NET
#ARCH = GCCOSX64
#ARCH = GCC64

CFLAGS = -W3  -I.\inc -nologo -D $(ARCH) -D _CRT_SECURE_NO_WARNINGS -TP
EXE_NAME = os345.exe

## To compile your code, be sure to place it under the OBJS variable
## and change the prefix to "obj" instead of "obj-ref".
## Every other object file MUST be placed in the REFOBJS variable.  If
## an object file is not located in either OBJS or REFOBJS, the linker
## will complain about missing references.  Here is a list of all object
## files:
OBJSREF	=			obj-ref\$(ARCH)\shell.o					\
					obj-ref\$(ARCH)\delta_clock.o			\
					obj-ref\$(ARCH)\semaphores.o			\
					obj-ref\$(ARCH)\scheduler.o				\
					obj-ref\$(ARCH)\queue_list.o			\
					obj-ref\$(ARCH)\jurassic_park.o			\
					obj-ref\$(ARCH)\virtual_memory.o		\
					obj-ref\$(ARCH)\fat.o					\

OBJSREFSWAP	=		obj-ref\$(ARCH)\shell-swap.o				\
					obj-ref\$(ARCH)\delta_clock-swap.o			\
					obj-ref\$(ARCH)\semaphores-swap.o			\
					obj-ref\$(ARCH)\scheduler-swap.o			\
					obj-ref\$(ARCH)\queue_list-swap.o			\
					obj-ref\$(ARCH)\jurassic_park-swap.o		\
					obj-ref\$(ARCH)\virtual_memory-swap.o		\
					obj-ref\$(ARCH)\fat-swap.o					\

O = .\obj
S = .\src

OBJS	=			$O\kernel.o								\
					$O\shell.o								\
					$O\signals.o							\
					$O\trap.o								\
					$O\system_calls.o						\
					$O\system_calls_kernel.o				\
					$O\delta_clock.o						\
					$O\delta_clock_tasks.o					\
					$O\semaphores.o							\
					$O\scheduler.o							\
					$O\messages.o							\
					$O\queue_list.o							\
					$O\park_interface.o						\
					$O\jurassic_park.o						\
					$O\vm_tasks.o							\
					$O\virtual_memory.o						\
					$O\lc3_simulator.o						\
					$O\fat.o								\
					$O\fat_tasks.o							\
					$O\my_tasks.o							\
					$O\semaphores_tasks.o					\
					$O\commands.o

all:		exe

{$S}.c{$O}.o: 
		 $(CC) $(CFLAGS) -Fo$@ -c $<

exe:		$(OBJS)
			link -out:$(EXE_NAME) $(OBJS) $(REFOBJS)

## A rule to clean up all the temporary files that can be
## generated in the development process.
## Usage: make clean
clean:
	 del obj\*.o  $(EXE_NAME)
