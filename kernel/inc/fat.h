// fat.h - file management system equates
#ifndef FAT_H
#define FAT_H

#include "type_defs.h"

// ***************************************************************************************

#define SECTORS_PER_DISK	2880
#define BYTES_PER_SECTOR   512
#define NUM_FAT_SECTORS		9
#define BEG_ROOT_SECTOR    19
#define BEG_DATA_SECTOR    33
#define CLUSTERS_PER_DISK  SECTORS_PER_DISK-BEG_DATA_SECTOR

#define BUFSIZE      512
#define NFILES       32					// # of valid open files

#define READ_ONLY 0x01
#define HIDDEN    0x02
#define SYSTEM    0x04
#define VOLUME    0x08 					// this is the volume label entry
#define DIRECTORY 0x10
#define ARCHIVE   0x20 					// same as file
#define LONGNAME  (READ_ONLY | HIDDEN | SYSTEM | VOLUME)

#define OPEN_READ		0				// read only
#define OPEN_WRITE	1					// write only
#define OPEN_APPEND	2					// append
#define OPEN_RDWR		3				// read/write

#define ENTRIES_PER_SECTOR 16
#define FAT_EOC   4095
#define FAT_BAD   4087
#define C_2_S(c) (c+BEG_DATA_SECTOR-2)
#define S_2_C(s) (s-BEG_DATA_SECTOR+2)

#define FILE_ALTERED       0x80
#define BUFFER_ALTERED     0x40

static const int FATDEBUG = 0;

typedef struct
{	unsigned short	free;	         	// # of sectors free
   unsigned short	used;	         	// # of sectors used
   unsigned short	bad;	         	// # of bad sectors
   unsigned short	size;	         	// Total # of sectors in RAM disk
} DiskSize;


#pragma pack(push,1)						// BYTE align in memory (no padding)
typedef struct {
	unsigned char	name[8];				// file name
	unsigned char	extension[3];			// extension
	unsigned char	attributes;				// file attributes code
	unsigned short directoryCluster;		// starting cluster of containing directory
	unsigned short	startCluster;			// first cluster of the file
	unsigned short	currentCluster;			// current cluster in buffer
	unsigned int	fileSize;							// file size in bytes
	int pid;								// process who opened file
	char mode;								// access mode (read, read-only, write, append)
	char flags;								// flags
												//   x80 = file altered
												//   x40 = buffer altered
												//   x20 = locked
												//   x10 =
												//   x08 = write protected
												//   x04 = contiguous
												//   x02 =
												//   x01 =
	unsigned long fileIndex;				// next character position (from beg of file)
	char buffer[BYTES_PER_SECTOR];			// file buffer
} FDEntry;
#pragma pack(pop)						// End of strict alignment

#pragma pack(push,1)					// BYTE align in memory (no padding)
typedef struct {
	unsigned char	BS_jmpBoot[3];		// Jump instruction to the boot code
	unsigned char 	BS_OEMName[8];		// Name of system that formatted the volume
	unsigned short	BPB_BytsPerSec;		// How many bytes in a sector (should be 512)
	unsigned char	BPB_SecPerClus;		// How many sectors are in a cluster (1)
	unsigned short	BPB_RsvdSecCnt;		// Number of sectors that are reserved (1)
	unsigned char	BPB_NumFATs;		// The number of FAT tables on the disk (2)
	unsigned short	BPB_RootEntCnt;		// Maximum # of directory entries in root directory
	unsigned short	BPB_TotSec16;		// FAT-12 total number of sectors on the disk
	unsigned char	BPB_Media;			// Code for media type {fixed, removable, etc.}
	unsigned short	BPB_FATSz16;		// FAT-12 # of sectors that each FAT occupies (9)
	unsigned short	BPB_SecPerTrk;		// Number of sectors in one cylindrical track
	unsigned short	BPB_NumHeads;		// Number of heads (2 heads for 1.4Mb 3.5" floppy)
	unsigned int	BPB_HiddSec;		// Number of preceding hidden sectors (0)
	unsigned int	BPB_TotSec32;		// FAT-32 number of sectors on the disk (0)
	unsigned char	BS_DrvNum;			// A drive number for the media (OS specific)
	unsigned char	BS_Reserved1;		// Reserved space for Windows NT (set to 0)
	unsigned char	BS_BootSig;			// Indicates following 3 fields are present (0x29)
	unsigned int	BS_VolID;			// Volume serial number (for tracking this disk)
	unsigned char	BS_VolLab[11];		// Volume label (matches rdl or "NO NAME    ")
	unsigned char	BS_FilSysType[8];	// Deceptive FAT type Label
	unsigned char	BS_fill[450];
} BSStruct;
#pragma pack(pop)						// End of strict alignment


// this struct may need to change for big endian
#pragma pack(push,1)					// BYTE align in memory (no padding)
typedef struct {						// (total 16 bits--a unsigned short)
	unsigned short sec: 5;				// low-order 5 bits are the seconds
	unsigned short min: 6;				// next 6 bits are the minutes
	unsigned short hour: 5;				// high-order 5 bits are the hour
} FATTime;
#pragma pack(pop)						// End of strict alignment


// this struct may need to change for big endian
#pragma pack(push,1)					// BYTE align in memory (no padding)
typedef struct {												// (total 16 bits--a unsigned short)
   unsigned short day: 5;				// low-order 5 bits are the day
   unsigned short month: 4;				// next 4 bits are the month
   unsigned short year: 7;				// high-order 7 bits are the year
} FATDate;
#pragma pack(pop)						// End of strict alignment

#pragma pack(push,1)					// BYTE align in memory (no padding)
typedef struct {
	unsigned char	name[8];	      	// File name
	unsigned char	extension[3];		// Extension
	unsigned char	attributes;			// Holds the attributes code
	unsigned char	reserved[10];		// Reserved
	FATTime time;			         	// Time of last write
	FATDate date;			         	// Date of last write
	unsigned short	startCluster;		// Pointer to the first cluster of the file.
	unsigned int	fileSize;			// File size in bytes
} DirEntry;
#pragma pack(pop)						// End of strict alignment

typedef struct {
   int error;
   char error_msg[32];
} FMSERROR;


// ***************************************************************************************
//	Prototypes
//
void dumpRAMDisk(char*, int, int);
void printDirectoryEntry(DirEntry*);
void printFatEntries(unsigned char*, int, int);

unsigned short getFatEntry(int, unsigned char*);
void setFatEntry(int, unsigned short);

int fmsChangeDir(char*);
int fmsGetNextDirEntry(int*, char*, DirEntry*, int);
int fmsCloseFile(int);
int fmsDefineFile(char*, int);
int fmsDeleteFile(char*);
int fmsOpenFile(char*, int);
int fmsReadFile(int, char*, int);
int fmsSeekFile(int, int);
int fmsWriteFile(int, char*, int);

int fmsLoadFile(char*, void*, int);
int fmsMount(char*, void*);
int fmsReadSector(void*, int);
int fmsWriteSector(void*, int);
int fmsUnMount(char*, void*);
void fmsError(int);

void setDirTimeDate(DirEntry*);
int fmsMask(char*,unsigned char*,unsigned char*);
int fmsGetDirEntry(char*, DirEntry*);
int advanceString(unsigned char*, int, char);
void convertToUpperCase(char*);
void removeFolderName(char*);
void getEntryStrings(DirEntry*,unsigned char*,unsigned char*);
void getFDEntryStrings(FDEntry*,unsigned char*,unsigned char*);
int validateAndParse(bool,char*,char*,char*);
int getEndCluster(int,int);
int getAvailableCluster();
int assignToEntry(DirEntry*,char*,char*,int,int);
int getNumOfClusters(int,int);
int checkForDirectoryEntries(int);

// ***************************************************************************************
//	FMS Errors

#define NUM_ERRORS   25

#define E_INVALID_FILE_NAME		        -50
#define E_INVALID_FILE_TYPE     		-51
#define E_INVALID_FILE_DESCRIPTOR  		-52
#define E_INVALID_SECTOR_NUMBER 		-53
#define E_INVALID_FAT_CHAIN     		-54
#define E_INVALID_DIRECTORY     		-55

#define E_FILE_ALREADY_DEFINED  		-60
#define E_FILE_NOT_DEFINED      		-61
#define E_FILE_ARLEADY_OPEN     		-62
#define E_FILE_NOT_OPEN         		-63
#define E_FILE_DIRECTORY_FULL   		-64
#define E_FILE_SPACE_FULL       		-65
#define E_END_OF_FILE           		-66
#define E_END_OF_DIRECTORY      		-67
#define E_DIRECTORY_NOT_FOUND   		-68
#define E_CAN_NOT_DELETE        		-69

#define E_TOO_MANY_FILES_OPEN   		-70
#define E_NOT_ENOUGH_CONTINOUS_SPACE    -71
#define E_DISK_NOT_MOUNTED      		-72

#define E_FILE_SEEK_ERROR       		-80
#define E_FILE_LOCKED   		        -81
#define E_FILE_DELETE_PROTECTED 		-82
#define E_FILE_WRITE_PROTECTED  		-83
#define E_READ_ONLY_FILE        		-84
#define E_ILLEGAL_ACCESS		        -85

#define E_UNDEFINED	                    -1

#define E_INVALID_FILE_NAME_MSG          "Invalid File Name"
#define E_INVALID_FILE_TYPE_MSG          "Invalid File Type"
#define E_INVALID_FILE_DESCRIPTOR_MSG    "Invalid File Descriptor"
#define E_INVALID_SECTOR_NUMBER_MSG      "Invalid Sector Number"
#define E_INVALID_FAT_CHAIN_MSG          "Invalid FAT Chain"
#define E_INVALID_DIRECTORY_MSG          "Invalid Directory"

#define E_FILE_ALREADY_DEFINED_MSG       "File Already Defined"
#define E_FILE_NOT_DEFINED_MSG           "File Not Defined"
#define E_FILE_ARLEADY_OPEN_MSG          "File Already Open"
#define E_FILE_NOT_OPEN_MSG              "File Not Open"
#define E_FILE_DIRECTORY_FULL_MSG        "File Directory Full"
#define E_FILE_SPACE_FULL_MSG            "File Space Full"
#define E_END_OF_FILE_MSG                "End-Of-File"
#define E_END_OF_DIRECTORY_MSG           "End-Of-Directory"
#define E_DIRECTORY_NOT_FOUND_MSG        "Directory Not Found"
#define E_CAN_NOT_DELETE_MSG             "Can Not Delete"

#define E_TOO_MANY_FILES_OPEN_MSG        "Too Many Files Open"
#define E_NOT_ENOUGH_CONTINOUS_SPACE_MSG "Not Enough Contiguous Space"
#define E_DISK_NOT_MOUNTED_MSG           "Disk Not Mounted"

#define E_FILE_SEEK_ERROR_MSG            "File Seek Error"
#define E_FILE_LOCKED_MSG                "File Locked"
#define E_FILE_DELETE_PROTECTED_MSG      "File Delete Protected"
#define E_FILE_WRITE_PROTECTED_MSG       "File Write Protected"
#define E_READ_ONLY_FILE_MSG             "Read Only File"
#define E_ILLEGAL_ACCESS_MSG             "Illegal Access"
#define E_TOTALLY_PWNED_MSG              "n00b got pwn3d"

#define E_UNDEFINED_MSG                  "Undefined Error"

#endif // __os345fat_h__
