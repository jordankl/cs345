#ifndef VM_TASKS_H
#define VM_TASKS_H


// TODO: Do we want to make these "tasks"?
int dumpLC3Mem(int argc, char* argv[]);
int vmaccess(int argc, char* argv[]);
int dumpPageMemory(int argc, char* argv[]);
int initMemory(int argc, char* argv[]);
int dumpVirtualMem(int argc, char* argv[]);
int virtualMemStats(int argc, char* argv[]);
int dumpFrameTable(int argc, char* argv[]);
int dumpPageTable(int argc, char* argv[]);
int dumpFrame(int argc, char* argv[]);
int rootPageTable(int argc, char* argv[]);
int userPageTable(int argc, char* argv[]);
int crawler(int argc, char* argv[]);
int memtest(int argc, char* argv[]);
int vm_test( int argc, char* argv[]);

// Help Functions
void displayFrame(int f);
void displayRPT(int rptNum);
void displayUPT(int rptNum, int uptNum);
void outPTE(char* s, int pte);
void displayPT(int pta, int badr, int inc);
void lookVM(int va);
void displayPage(int pn);
void dumpMemory(char *s, int sa, int ea);
void dumpVMemory(char *s, int sa, int ea);
void loadLC3File(char* string);


#endif
