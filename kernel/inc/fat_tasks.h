#ifndef FAT_TASKS_H
#define FAT_TASKS_H

#include "fat.h"
#include "type_defs.h"

int fat_test(int argc, char* argv[]);
int fat_cd(int argc, char* argv[]);
int fat_dir(int argc, char* argv[]);
int fat_dfat(int argc, char* argv[]);
int fat_mount(int argc, char* argv[]);
int fat_run(int argc, char* argv[]);
int fat_type(int argc, char* argv[]);
int fat_dumpSector(int argc, char* argv[]);
int fat_fileSlots(int argc, char* argv[]);
int fat_copy(int argc, char* argv[]);
int fat_define(int argc, char* argv[]);
int fat_del(int argc, char* argv[]);
int fat_mkdir(int argc, char* argv[]);
int fat_unmount(int argc, char* argv[]);
int fat_open(int argc, char* argv[]);
int fat_read(int argc, char* argv[]);
int fat_write(int argc, char* argv[]);
int fat_seek(int argc, char* argv[]);
int fat_close(int argc, char* argv[]);
int fat_chkdsk(int argc, char* argv[]);

// Support Functions
void outFatEntry(int index);
void printDirectoryEntry(DirEntry* dirent);
void printFatEntries(unsigned char* FAT, int start, int end);
void dumpRAMDisk(char *s, int sa, int ea);
int isValidDirEntry(DirEntry* dirEntry);
void getFileName(char* fileName, DirEntry* dirEntry);
void checkDirectory(char* dirName, unsigned char fat[], int dir);
int fmsTests(int test, bool debug);


#endif
