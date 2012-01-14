// Jurassic Park
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
#include <time.h>
#include <assert.h>
#include "jurassic_park.h"
#include "kernel.h"
#include "delta_clock.h"
#include "semaphores.h"
#include "system_calls.h"
#include "signals.h"

//#define DEBUG_ON TRUE
#ifdef DEBUG_ON
#define DEBUG_PRINT(fmt,...) printf(fmt,__VA_ARGS__);
#else
#define DEBUG_PRINT(fmt,...)
#endif

// Given to me
extern JPARK myPark;
extern int parkMutexSemId;						// protect park access
extern int jurassicDisplaySemId;
extern int fillSeatSemId[NUM_CARS];			// (signal) seat ready to fill
extern int seatFilledSemId[NUM_CARS];			// (wait) passenger seated
extern int rideOverSemId[NUM_CARS];			// (signal) ride over
extern int visitors;



void initializeGlobalVariables() {
}

// ***********************************************************************
// ***********************************************************************
// carTask
int carTask(int argc, char* argv[])
{

    // while there are still people in the park
	while (myPark.numExitedPark < visitors) {

        swapTask();

	}

	return 0;

} // end carTask()

// ***********************************************************************
// ***********************************************************************
// visitorTask(int argc, char* argv[])
int visitorTask(int argc, char* argv[])
{
	return 0;
} // end visitorTask

// ***********************************************************************
// ***********************************************************************
// driverTask
int driverTask(int argc, char* argv[])
{
	while (TRUE) {

        swapTask();

	}
	return 0;
} // end driverTask

/**
*  Use this function to clean up any heap memory
*  after the park has finished.  It will be called by
*  the park interface (park_interface.c).
*/
void parkCleanup() {
}
