#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

// Swap space
#define PAGE_FREE			3
#define PAGE_NEW_WRITE		2
#define PAGE_OLD_WRITE		1
#define PAGE_READ			0
#define PAGE_INIT			-1

#define MMUDEBUG 0


/**
 * struct RPTTable
 * @rptAddress: the address of the RPTTable
 * @allocated: this is used to determine if the table is allocated in memory
 */
typedef struct {
	int rptAddress;
	int allocated;
} RPTTable;

void initVM();
int getFreeRPT();
unsigned short int *getMemAdr(int va, int rwFlg);
void setFrameTableBits(int flg, int sf, int ef);
int getAvailableFrame();
int swapFrame();
int checkForPages(int uptAddr);
void movePointers();
void freeVirtualMemory(int taskId);
void freeFrame(int frame);
void printAllFrames();
int accessPage(int pnum, int frame, int rwnFlg);
unsigned short int* getPageAddress(int pnum);


#endif
