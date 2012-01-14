// LC-3 Memory Management Unit
// **************************************************************************
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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "kernel.h"
#include "lc3_simulator.h"
#include "virtual_memory.h"


//#define DEBUG

#ifdef DEBUG
#define DEBUGPRINT(...)       printf(__VA_ARGS__)
#else
#define DEBUGPRINT(...)
#endif

// We do not want to be setting frame table bits and then take a context
// switch, so we will lock that data structure by disabling swaps until
// we are done with any particular table operation.

// ***********************************************************************
// mmu variables


// LC-3 memory
unsigned short int memory[LC3_MAX_MEMORY];
unsigned short int swapMemory[LC3_MAX_SWAP_MEMORY];

// statistics
int memAccess;							// memory accesses
int memHits;							// memory hits
int memPageFaults;					// memory faults
int nextPage;							// swap page size
int pageReads;							// page reads
int pageWrites;						// page writes

int numRPT = 32;


RPTTable freeRPTTable[32];

// @DISABLE_SWAPS
void initVM() {
	int i = 0;
	int rptAddress = LC3_RPT;
	while (i < numRPT) {
		freeRPTTable[i].rptAddress = rptAddress;
		freeRPTTable[i].allocated = FALSE;
		rptAddress += LC3_FRAME_SIZE;
		++i;
	}

	// Add initialization code if needed
	// Called at startup and in initializeLC3memory
}

void releaseRPT(int rptAddress) {
}

int getFreeRPT() {
	int i = 0;
	while (i < numRPT) {
		if (freeRPTTable[i].allocated) {
			++i;
			continue;
		}
		freeRPTTable[i].allocated = TRUE;
		return freeRPTTable[i].rptAddress;
	}
	assert(FALSE);
	return 0;
}








// @ENABLE_SWAPS

// **************************************************************************
// **************************************************************************
// LC3 Memory Management Unit
// Virtual Memory Process
// **************************************************************************
//           ___________________________________Frame defined
//          / __________________________________Dirty frame
//         / / _________________________________Referenced frame
//        / / / ________________________________Pinned in memory
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023) (2^10)
//     / / / /     /                 / _________page defined
//    / / / /     /                 / /       __page # (0-4096) (2^12)
//   / / / /     /                 / /       /
//  / / / /     / 	             / /       /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p
//@DISABLE_SWAPS
unsigned short int *getMemAdr(int va, int rwFlg)
{
    unsigned short int pa = va;

	// turn off virtual addressing for system RAM
	if (va < 0x3000) {
		memAccess++;
		memHits++;
		return &memory[va];
	}

	return &memory[pa];
} // end getMemAdr
//@ENABLE_SWAPS



// **************************************************************************
// **************************************************************************
// set frames available from sf to ef
//    flg = 0 -> clear all others
//        = 1 -> just add bits
//@DISABLE_SWAPS
void setFrameTableBits(int flg, int sf, int ef)
{	int i, data;
	int adr = LC3_FBT-1;             /* index to frame bit table */
	int fmask = 0x0001;              /* bit mask */

	// 1024 frames in LC-3 memory
	for (i=0; i<LC3_FRAMES; i++)
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;
			adr++;
			data = (flg)?MEMWORD(adr):0;
		}
		else fmask = fmask >> 1;
		// allocate frame if in range
		if ( (i >= sf) && (i < ef)) data = data | fmask;
		MEMWORD(adr) = data;
	}
	return;
} // end setFrameTableBits
//@ENABLE_SWAPS



// **************************************************************************
// get frame from frame bit table (else return -1)
//@DISABLE_SWAPS
int getAvailableFrame()
{
	int i, data;
	int adr = LC3_FBT - 1;				/* index to frame bit table */
	int fmask = 0x0001;					/* bit mask */

	for (i=0; i<LC3_FRAMES; i++)		/* look thru all frames */
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;				/* move to next work */
			adr++;
			data = MEMWORD(adr);
		}
		else fmask = fmask >> 1;		/* next frame */
		// deallocate frame and return frame #
		if (data & fmask)
		{  MEMWORD(adr) = data & ~fmask;
			return i;
		}
	}
	return -1;
} // end getAvailableFrame
//@ENABLE_SWAPS



// **************************************************************************
// read/write to swap space
//
// By way of clarification, pnum and frame are treated as index values
// into an array of "frames" and an array of "pages".  The function
// handles all the shifting to turn the index value into an actual
// address into memory or swap space.  The intent is to simplify the
// interface.  You can, for example, swap frame 192 into page 1 on
// disk using 1 as the pnum and 192 as the frame and setting the
// appropriate rwnFlg to PAGE_OLD_WRITE.  Other options for rwnFlg are
// defined in os345lc3.h
extern jmp_buf reset_context;//added by Brandon 19Jan09
//@DISABLE_SWAPS
int accessPage(int pnum, int frame, int rwnFlg)
{
   if ((nextPage >= LC3_MAX_PAGE) || (pnum >= LC3_MAX_PAGE))
   {
      printf("\nVirtual Memory Space Exceeded!  (%d)", LC3_MAX_PAGE);
	  // +++++ egm 01-10-2009
	  // We need to give some meaning to -4 so we can better interpret
	  // exit codes.
	  // -----
     //exit(-4);
     longjmp( reset_context, POWER_DOWN_ERROR );//added by Brandon 19Jan09
   }
   switch(rwnFlg)
   {
      case PAGE_INIT:                    		// init paging
         nextPage = 0;
         return 0;

      case PAGE_NEW_WRITE:                   // new write (Drops thru to write old)
         pnum = nextPage++;

      case PAGE_OLD_WRITE:                   // write
         //printf("\n    (%d) Write frame %d (memory[%04x]) to page %d", p.PID, frame, frame<<6, pnum);
         memcpy(&swapMemory[pnum<<6], &memory[frame<<6], 1<<7);
         pageWrites++;
         return pnum;

      case PAGE_READ:                    // read
         //printf("\n    (%d) Read page %d into frame %d (memory[%04x])", p.PID, pnum, frame, frame<<6);
      	memcpy(&memory[frame<<6], &swapMemory[pnum<<6], 1<<7);
         pageReads++;
         return pnum;

      case PAGE_FREE:                   // free page
         break;
   }
   return pnum;
} // end accessPage
//@ENABLE_SWAPS

//@DISABLE_SWAPS
unsigned short int* getPageAddress(int pnum) {
    return &swapMemory[pnum<<6];
}
//@ENABLE_SWAPS
