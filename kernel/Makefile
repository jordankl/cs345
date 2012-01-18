## -------------------------------------------------------------------
##
## Author:              Eric Mercer
## Date Created:        Feb. 20, 2003
## Last Modified:       Nov. 16, 2009 by Trevor
##
## Skeleton Makefile
##
## -------------------------------------------------------------------

## Boiler plate definitions

## CC specifies the compiler to use.  It can be set to CC, gcc, or g++
## on most Linux systems.  We are going to make gcc the standard
## compiler for this class.
CC      =       gcc

CP		=		cp

## SWAP Tool for inserting swap directives
SWAPTOOL = SwapInserter.pl

## CFLAGS are the compiler options.
##
##   -Wall: maximum warning level
##   -g: compile with debug symbols for gdb
##
## See the manpage for gcc for other options (i.e. -O, -p, etc.)
##

##CFLAGS = -Wall -g -m32
CFLAGS = -Wall -g

## If you need a separate directory for include files, then you
## can specify the directory here.
INCLUDE = 	-Iinc

## Specific defines for the build process
## directives
#ARCH = GCCW32
#ARCH = GCCOSX64
ARCH = GCC64

DEFINE = -D$(ARCH) -DKDEBUG=FALSE


EXE_NAME = os345
SWAP_EXE_NAME = os345-swp

## files with white space (i.e., main.o foo.o).  If you need continue
## on a new line, then add a '\' character to end the current line.
## For example:
##
## OBJS = main.o \
##        foo.o
##

OBJSREF	=			obj-ref/$(ARCH)/shell.o					\
					obj-ref/$(ARCH)/delta_clock.o			\
					obj-ref/$(ARCH)/semaphores.o			\
					obj-ref/$(ARCH)/scheduler.o				\
					obj-ref/$(ARCH)/queue_list.o			\
					obj-ref/$(ARCH)/jurassic_park.o			\
					obj-ref/$(ARCH)/virtual_memory.o		\
					obj-ref/$(ARCH)/fat.o					\

OBJSREFSWAP	=		obj-ref/$(ARCH)/shell-swap.o			\
					obj-ref/$(ARCH)/delta_clock-swap.o		\
					obj-ref/$(ARCH)/semaphores-swap.o		\
					obj-ref/$(ARCH)/scheduler-swap.o		\
					obj-ref/$(ARCH)/queue_list-swap.o		\
					obj-ref/$(ARCH)/jurassic_park-swap.o	\
					obj-ref/$(ARCH)/virtual_memory-swap.o	\
					obj-ref/$(ARCH)/fat-swap.o				\

## To include your solution, change the appropriate reference
## below from obj-ref/${ARCH) to obj; otherwise, the linker
## will use teh reference objects.
OBJS	=			obj/kernel.o							\
					obj/signals.o							\
					obj/trap.o								\
					obj/system_calls.o						\
					obj/system_calls_kernel.o				\
					obj/delta_clock_tasks.o					\
					obj/messages.o							\
					obj/park_interface.o					\
					obj/vm_tasks.o							\
					obj/lc3_simulator.o						\
					obj/fat_tasks.o							\
					obj/my_tasks.o							\
					obj/semaphores_tasks.o					\
					obj/commands.o							\
				 	obj-ref/$(ARCH)/shell.o					\
					obj-ref/$(ARCH)/delta_clock.o			\
					obj-ref/$(ARCH)/semaphores.o			\
					obj-ref/$(ARCH)/scheduler.o				\
					obj/queue_list.o			\
					obj-ref/$(ARCH)/jurassic_park.o			\
					obj-ref/$(ARCH)/virtual_memory.o		\
					obj-ref/$(ARCH)/fat.o					\


SWAPOBJS	=		obj/kernel-swap.o						\
					obj/signals-swap.o						\
					obj/trap-swap.o							\
					obj/system_calls-swap.o					\
					obj/system_calls_kernel-swap.o			\
					obj/delta_clock_tasks-swap.o			\
					obj/messages-swap.o						\
					obj/park_interface-swap.o				\
					obj/vm_tasks-swap.o						\
					obj/lc3_simulator-swap.o				\
					obj/fat_tasks-swap.o					\
					obj/my_tasks-swap.o						\
					obj/semaphores_tasks-swap.o				\
					obj/commands-swap.o	                    \
					obj-ref/$(ARCH)/shell-swap.o			\
					obj-ref/$(ARCH)/delta_clock-swap.o		\
					obj-ref/$(ARCH)/semaphores-swap.o		\
					obj-ref/$(ARCH)/scheduler-swap.o		\
					obj-ref/$(ARCH)/queue_list-swap.o		\
					obj-ref/$(ARCH)/jurassic_park-swap.o	\
					obj-ref/$(ARCH)/virtual_memory-swap.o	\
					obj-ref/$(ARCH)/fat-swap.o				\


## -------------------------------------------------------------------
##
## Rule format:
##
## name: dependencies
##       command
##
## name: what you want to build
## dependencies: any file that 'name' depends on to build (i.e.,
##               object files and include files for an executable)
## command: what command are executed to build 'name'
##
## WARNING: be sure to uses tabs and not spaces because
##          spaces can confuse make.
##
## As an example, consider the following rules:
##
## hello: 		main.o
##			${CC} ${DEFINE} ${CFLAGS} ${INCLUDE} -o $@ $<
##
## Generic rule that knows how to create a .o file from any .c file.
## Notice that it does not include other dependencies so you may
## want to be more specific on the build rule.
##
## FYI, the $< refers to the first dependency on the dependency list
## More information in variables:
## http://theoryx5.uwinnipeg.ca/gnu/make/make_98.html#SEC97
##
## %.o:    	%.c
##		${CC} ${DEFINE} ${CFLAGS} ${INCLUDE} -c $<
##
## The first rule is the default rule.  At the command prompt, if
## you type make, then you get the following for this example:
##
## > make
## gcc  -Wall -g -I../include -c main.c
## gcc  -Wall -g -I../include -o hello main.o
##
## This executed the rule all that depends on hello that depends on
## main.o; thus, it builds main.o and then links it to create the
## hello program.

all:		exe

exe:		${OBJS}
			${CC} ${CFLAGS} ${INCLUDE} -o ${EXE_NAME} ${OBJS}

swap:		${SWAPOBJS}
			${CC} ${DEFINE} ${CFLAGS} ${INCLUDE} -o ${SWAP_EXE_NAME} ${SWAPOBJS}

src-swap/%-swap.c:	src/%.c inc/kernel.h src/kernel.c
			perl ${SWAPTOOL} $< -o$@ -t4 -f1 -s

obj/%.o:	src/%.c inc/kernel.h src/kernel.c
			${CC} ${DEFINE} ${CFLAGS} -o $@ ${INCLUDE} -c $<

obj/%-swap.o:    	src-swap/%-swap.c inc/kernel.h src/kernel.c
			${CC} ${DEFINE} ${CFLAGS} -o $@ ${INCLUDE} -c $<

obj-copy:	${OBJS} ${SWAPOBJS}
			cp obj/shell.o obj-ref/$(ARCH)/shell.o
			cp obj/delta_clock.o obj-ref/$(ARCH)/delta_clock.o
			cp obj/semaphores.o obj-ref/$(ARCH)/semaphores.o
			cp obj/scheduler.o obj-ref/$(ARCH)/scheduler.o
			cp obj/queue_list.o obj-ref/$(ARCH)/queue_list.o
			cp obj/jurassic_park.o obj-ref/$(ARCH)/jurassic_park.o
			cp obj/virtual_memory.o obj-ref/$(ARCH)/virtual_memory.o
			cp obj/fat.o obj-ref/$(ARCH)/fat.o
			cp obj/shell-swap.o obj-ref/$(ARCH)/shell-swap.o
			cp obj/delta_clock-swap.o obj-ref/$(ARCH)/delta_clock-swap.o
			cp obj/semaphores-swap.o obj-ref/$(ARCH)/semaphores-swap.o
			cp obj/scheduler-swap.o obj-ref/$(ARCH)/scheduler-swap.o
			cp obj/queue_list-swap.o obj-ref/$(ARCH)/queue_list-swap.o
			cp obj/jurassic_park-swap.o obj-ref/$(ARCH)/jurassic_park-swap.o
			cp obj/virtual_memory-swap.o obj-ref/$(ARCH)/virtual_memory-swap.o
			cp obj/fat-swap.o obj-ref/$(ARCH)/fat-swap.o

## A rule to clean up all the temporary files that can be
## generated in the development process.
## Usage: make clean
clean:
	 rm -f src-swap/*.c obj/*.o  ${EXE_NAME} ${SWAP_EXE_NAME}

refclean:
	rm -f obj-ref/$(ARCH)/*.o
