// Virtual Memory
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
#include "vm_tasks.h"
#include "virtual_memory.h"
#include "lc3_simulator.h"
#include "kernel.h"

// ***********************************************************************
// project 5 variables
extern int memAccess;
extern int memHits;
extern int memPageFaults;
extern int nextPage;
extern int pageReads;
extern int pageWrites;

extern unsigned short int memory[];
extern int getMemoryData(int);

// **************************************************************************
// **************************************************************************
// ---------------------------------------------------------------------
//           ___________________________________Frame defined
//          / __________________________________Dirty frame
//         / / _________________________________Referenced frame
//        / / / ________________________________Pinned in memory
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023)
//     / / / /     /                 /__________Swap page defined
//    / / / /     /                 //        __page # (0-4096)
//   / / / /     /                 //        /
//  / / / /     / 	             //        /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p

// ---------------------------------------------------------------------
// **************************************************************************
// dm <start_addr> <end_addr>
int dumpLC3Mem(int argc, char* argv[])
{
	int start_addr, end_addr;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <start_addr> <end_addr>\n", argv[0]);
		return 1;
    }

	start_addr = INTEGER(argv[1]);
	end_addr = start_addr + 0x0040;

	dumpMemory("LC-3 Memory", start_addr, end_addr);
	return 0;
}



// **************************************************************************
// **************************************************************************
// vma <address>
int vmaccess(int argc, char* argv[])
{
	// VMA will only use the shells RPT
	TCB* tcb = getTCB();
	int curTask = gettid();
	int SHELL_RPT = tcb[0].RPT;
	unsigned short int address, rpt, upt;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <address>\n", argv[0]);
		return 1;
    }

	address = INTEGER(argv[1]);

	tcb[curTask].RPT = SHELL_RPT;
#if defined(GCCW32) || defined(GCCOSXX86)
	printf(" = 0x%04x", getMemAdr(address, 1)-&MEMWORD(0));
#else
	printf(" = 0x%04lx", getMemAdr(address, 1)-&MEMWORD(0));
#endif
	for (rpt = 0; rpt < 64; rpt+=2) {
		if (MEMWORD(rpt+SHELL_RPT) || MEMWORD(rpt+SHELL_RPT+1)) {
			outPTE("  RPT  =", rpt+SHELL_RPT);
			for(upt = 0; upt < 64; upt+=2) {
				if (DEFINED(MEMWORD(rpt+SHELL_RPT)) &&
					(DEFINED(MEMWORD((FRAME(MEMWORD(rpt+SHELL_RPT))<<6)+upt))
					|| PAGED(MEMWORD((FRAME(MEMWORD(rpt+SHELL_RPT))<<6)+upt+1)))) {
					outPTE("    UPT=", (FRAME(MEMWORD(rpt+SHELL_RPT))<<6)+upt);
				}
			}
		}
	}
	printf("\nPages = %d\n", nextPage);
	return 0;
}



// **************************************************************************
// **************************************************************************
// pm <page #>  Display page frame
int dumpPageMemory(int argc, char* argv[])
{
	int page;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <page #>\n", argv[0]);
		return 1;
    }

	page = INTEGER(argv[1]);

	displayPage(page);

	return 0;
}



// **************************************************************************
// **************************************************************************
// im <a>  Initialize LC-3 memory bound
int initMemory(int argc, char* argv[])
{
	int highAdr = 0x8000;

	if (argc > 1) highAdr = INTEGER(argv[1]);
	if (highAdr < 0x3000) highAdr = (highAdr<<6) + 0x3000;
	if (highAdr > 0xf000) highAdr = 0xf000;
	printf("Setting upper memory limit to 0x%04x\n", highAdr);

	// init LC3 memory
	initLC3Memory(LC3_MEM_FRAME, highAdr>>6);
	printf("Physical Address Space = %d frames (%0.1fkb)\n",
         (highAdr>>6)-LC3_MEM_FRAME, ((highAdr>>6)-LC3_MEM_FRAME)/8.0);

	memAccess = 0;							/* vm statistics */
	memHits = 0;
	memPageFaults = 0;
	nextPage = 0;
	pageReads = 0;
	pageWrites = 0;

	return 0;
}

// **************************************************************************
// **************************************************************************
// dvm <start_addr>,<end_addr>
int dumpVirtualMem(int argc, char* argv[])	// dump virtual lc-3 memory
{
	int start_addr, end_addr;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <start_addr> <end_addr>\n", argv[0]);
		return 1;
    }

	start_addr = INTEGER(argv[1]);
	end_addr = start_addr + 0x0040;

	dumpVMemory("LC-3 Virtual Memory", start_addr, end_addr);
	lookVM(start_addr);
	return 0;
}

// **************************************************************************
// **************************************************************************
// vms
int virtualMemStats(int argc, char* argv[])
{
	double missRate;
	missRate = (memAccess)?(((double)memPageFaults)/(double)memAccess)*100.0:0;
	printf("\nMemory accesses = %d", memAccess);
	printf("\n           hits = %d", memHits);
	printf("\n         faults = %d", memPageFaults);
	printf("\n           rate = %f%%", missRate);
	printf("\n     Page reads = %d", pageReads);
	printf("\n    Page writes = %d", pageWrites);
	printf("\nSwap page count = %d (%d kb)", nextPage, nextPage>>3);
	return 0;
}

// **************************************************************************
// **************************************************************************
// dft
int dumpFrameTable(int argc, char* argv[])
{
	dumpMemory("Frame Bit Table", LC3_FBT, LC3_FBT+0x40);
	return 0;
}

// **************************************************************************
// **************************************************************************
// dft
int dumpPageTable(int argc, char* argv[])
{
	dumpMemory("Frame Bit Table", LC3_PBT, LC3_PBT+0x100);
	return 0;
}

// **************************************************************************
// **************************************************************************
// dfm <frame>
int dumpFrame(int argc, char* argv[])
{
	int frame;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <frame #>\n", argv[0]);
		return 1;
    }

	frame = INTEGER(argv[1]);

	displayFrame(frame%LC3_FRAMES);
	return 0;
}

// **************************************************************************
// **************************************************************************
// rpt <task #>       Display process root page table
int rootPageTable(int argc, char* argv[])
{
	int task;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <task #>\n", argv[0]);
		return 1;
    }

	task = INTEGER(argv[1]);

	displayRPT(task);
	return 0;
}

// **************************************************************************
// **************************************************************************
// upt <rpt==task> <upt>    Display process user page table
int userPageTable(int argc, char* argv[])
{
	int task, upt;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <task #> <addr>\n", argv[0]);
		return 1;
    }

	task = INTEGER(argv[1]);
	upt = INTEGER(argv[2]);

	displayUPT(task, upt);
	return 0;
}

// **************************************************************************
// **************************************************************************
void displayFrame(int f)
{
	char mesg[128];
	sprintf(mesg, "Frame %d", f);
	dumpMemory(mesg, f*LC3_FRAME_SIZE, (f+1)*LC3_FRAME_SIZE);
	return;
}

// **************************************************************************
// **************************************************************************
// display contents of RPT rptNum
void displayRPT(int rptNum)
{
	displayPT(LC3_RPT + (rptNum<<6), 0, 1<<11);
	return;
}

// **************************************************************************
// **************************************************************************
// display contents of UPT
void displayUPT(int rptNum, int uptNum)
{
	unsigned short int  rpte, upt, uptba;
	rptNum &= BITS_4_0_MASK;
	uptNum &= BITS_4_0_MASK;

	// index to process <rptNum>'s rpt + <uptNum> index
	rpte = MEMWORD(((LC3_RPT + (rptNum<<6)) + uptNum*2));
	// calculate upt's base address
	uptba = uptNum<<11;
	if (DEFINED(rpte)) {
		upt = FRAME(rpte)<<6;
	} else {  printf("\nUndefined!");
		return;
	}

	displayPT(upt, uptba, 1<<6);
	return;
}

// **************************************************************************
// **************************************************************************
// output page table entry
void outPTE(char* s, int pte)
{
	int pte1, pte2;
	char flags[8];

	// read pt
	pte1 = memory[pte];
	pte2 = memory[pte+1];

	// look at appropriate flags
	strcpy(flags, "-----");
	if (DEFINED(pte1)) flags[0] = 'F';
	if (DIRTY(pte1)) flags[1] = 'D';
	if (REFERENCED(pte1)) flags[2] = 'R';
	if (PINNED(pte1)) flags[3] = 'P';
	if (PAGED(pte2)) flags[4] = 'S';

	// output pte line
	printf("\n%s x%04x = %04x %04x  %s", s, pte, pte1, pte2, flags);
	if (DEFINED(pte1) || DEFINED(pte2)) printf(" Frame=%d", FRAME(pte1));
	if (DEFINED(pte2)) printf(" Page=%d", SWAPPAGE(pte2));

	return;
}

// **************************************************************************
// **************************************************************************
// display page table entries
void displayPT(int pta, int badr, int inc)
{
	int i;
	char buf[32];

	for (i=0; i<32; i++) {
      sprintf(buf, "(x%04x-x%04x) ", badr+ i*inc, badr + ((i+1)*inc)-1);
		outPTE("", (pta + i*2));
	}

   return;
}

// **************************************************************************
// **************************************************************************
// look at virtual memory location va
void lookVM(int va)
{
   unsigned short int rpte1, rpte2, upte1, upte2, pa;

   // get root page table entry
	rpte1 = MEMWORD(LC3_RPT + RPTI(va));
	rpte2 = MEMWORD(LC3_RPT + RPTI(va) + 1);
	if (DEFINED(rpte1)) {
		upte1 = MEMWORD((FRAME(rpte1)<<6) + UPTI(va));
		upte2 = MEMWORD((FRAME(rpte1)<<6) + UPTI(va) + 1);
	} else {
		// rpte undefined
		printf("\n  RTB[Undefined]");
		return;
	}

  	if (DEFINED(upte1)) {
		pa = (FRAME(upte1)<<6) + FRAMEOFFSET(va);
	} else {
		// upte undefined
		printf("\n  UTB[Undefined]");
		return;
	}

	printf("\n  RPT[0x%04x] = %04x %04x", LC3_RPT + RPTI(va), rpte1, rpte2);

	if (rpte1&BIT_14_MASK) printf(" D");
	if (rpte1&BIT_13_MASK) printf(" R");
	if (rpte1&BIT_12_MASK) printf(" P");

	printf(" Frame=%d", rpte1 & 0x03ff);

	if (DEFINED(rpte2)) printf(" Page=%d", rpte2 & 0x0fff);

	printf("\n  UPT[0x%04x] = %04x %04x", (FRAME(rpte1)<<6) + UPTI(va), upte1, upte2);

	if (upte1&BIT_14_MASK) printf(" D");
	if (upte1&BIT_13_MASK) printf(" R");
	if (upte1&BIT_12_MASK) printf(" P");

	printf(" Frame=%d", upte1 & 0x03ff);

	if (DEFINED(upte2)) printf(" Page=%d", upte2 & 0x0fff);

	printf("\n  MEM[0x%04x] = %04x", pa, MEMWORD(pa));

	return;
}

// **************************************************************************
// **************************************************************************
// pm <#>  Display page frame
void displayPage(int pn)
{
   unsigned short int *buffer;
   int i, ma;
   printf("\nPage %d", pn);
   buffer = getPageAddress(pn);
   for (ma = 0; ma < 64;)
	{
      printf("\n0x%04x:", ma);
		for (i=0; i<8; i++)
		{
		   printf(" %04x", MASKTO16BITS(buffer[ma + i]));
      }
		ma+=8;
   }
   return;
}

// **************************************************************************
// **************************************************************************
// dm <sa> <ea> - dump lc3 memory
void dumpMemory(char *s, int sa, int ea)
{
	int i, ma;
	printf("%s", s);
	for (ma = sa; ma < ea; ma += 8) {
		printf("\n0x%04x:", ma);
		for (i=0; i<8; i++) {
			printf(" %04x", MEMWORD((ma+i)));
		}
	}

	printf("\n");
	return;
}

// **************************************************************************
// **************************************************************************
// dvm <sa> <ea> - dump lc3 virtual memory
void dumpVMemory(char *s, int sa, int ea)
{
	int i, ma;
	printf("\n%s", s);
	for (ma = sa; ma < ea; ma += 8) {
		printf("\n0x%04x:", ma);
		for (i=0; i<8; i++) {
			printf(" %04x", getMemoryData(ma+i));
		}
	}

	return;
}

// **************************************************************************
// **************************************************************************
// crawler and memtest programs
void loadLC3File(char* string)
{
	char* myArgv[2];
	char buff[32];

	strcpy(buff, string);

	if (strchr(buff, '.')) {
		*(strchr(buff, '.')) = 0;
	}

	myArgv[0] = string;
	myArgv[1] = string;

	NewTask lc3TaskInfo;
	lc3TaskInfo.name = myArgv[0];
	lc3TaskInfo.task = lc3Task;
	lc3TaskInfo.priority = MED_PRIORITY;
	lc3TaskInfo.argc = 2;
	lc3TaskInfo.argv = myArgv;
	lc3TaskInfo.parentHandlers = FALSE;
	lc3TaskInfo.tgidNew = FALSE;
	createTask(&lc3TaskInfo);

	return;
}

//*****************************************************************************/
// testVM: runs 0+ memtests and 0+ crawlers
// Note: args not required
// MAXIMUM COMBINATIONS (for 2 frames):
// vmt <#mem> <#cra>
// vmt 5 0
// vmt 4 2
// vmt 3 4
// vmt 2 5
// vmt 1 7
// vmt 0 9
//*****************************************************************************/
#define MAX(x,y) ((x>y)?x:y)
int vm_test( int argc, char* argv[] )
{
	static int qtyM, qtyC, i, max; i = qtyM = qtyC = 0;
	if( argc > 1 ){
		qtyM = INTEGER( argv[1] );
	}

	if( argc > 2 ){
		qtyC = INTEGER( argv[2] );
	}

	printf( "Executing %d memtest(s) and %d crawler(s)\n", qtyM, qtyC );
	for( max = MAX( qtyM, qtyC ); i < max; ++i ) {
		if( i < qtyM ) {
			loadLC3File( "lc3-programs/memtest.hex" );
		}

		if( i < qtyC ) {
			loadLC3File( "lc3-programs/crawler.hex" );
		}
	}
	return 0;
}

//*****************************************************************************/
//crawler: runs 1+ crawlers
//Note: args not required
// MAXIMUM (for 2 frames): cra 9
//*****************************************************************************/
int crawler( int argc, char* argv[] )
{
	static int qty, i; qty = 1;
	if( argc > 1 && ( i = INTEGER( argv[1] ) ) ) {
		qty = i;
	}

	printf( "Executing %d crawler(s)\n", qty );

	for( i = 0; i < qty; ++i ) {
		loadLC3File("lc3-programs/crawler.hex");
	}
	return 0;
}

//*****************************************************************************/
//memtest: runs 1+ memtests
//Note: args not required
// MAXIMUM (for 2 frames): mem 5
//*****************************************************************************/
int memtest( int argc, char* argv[] ){
	static int qty, i; qty = 1;
	if( argc > 1 && ( i = INTEGER( argv[1] ) ) ) {
		qty = i;
	}

	printf( "Executing %d memtest(s)\n", qty );

	for( i = 0; i < qty; ++i ) {
		loadLC3File( "lc3-programs/memtest.hex" );
	}
	return 0;
}
