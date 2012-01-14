#include "commands.h"
#include "my_tasks.h"
#include "delta_clock_tasks.h"
#include "vm_tasks.h"
#include "fat_tasks.h"
#include "semaphores_tasks.h"
#include "shell.h"

int number_builtin_cmds = 9;
Command builtin_cmd_list[] = {
	{	"quit",			"q",		cmd_quit,				"Quit"},
	{	"jobs",			"jobs",		cmd_listjobs,			"List jobs"},
	{	"bg",			"bg",		cmd_do_bgfg,			"Background a job"},
	{	"fg",			"fg",		cmd_do_bgfg,			"Foreground a job"},
	{	"sigKill",		"sk",		cmd_sigKill,			"Send signal to task"},
	{	"help",			"he",		cmd_help,				"OS345 Help"},
	{	"tasks",		"lt",		cmd_listTasks,			"List tasks"},
	{	"sems",			"ls",		cmd_listSems,			"List Semaphores"},
	{	"reset",		"rs",		cmd_reset,				"Reset system"}
};

int number_commands = 45;
Command command_list[] = {

	// My Tasks - 4
	{	"myint",		"myint",	myint,					"Sleep <n> seconds then KSIGINT self"},
	{	"mysplit",		"mysplit",	mysplit,				"Create task and wait for task to terminate"},
	{	"mystop",		"mystop",	mystop,					"Sleep <n> seconds then KSIGTSTOP self"},
	{	"myspin",		"myspin",	myspin,					"Sleep for <n> seconds and exit"},

	// Delta Clock Task - 3
	{	"testsc",		"tsc",		testScheduler,			"Test scheduler"},
	{	"testdc",		"tdc",		testDeltaClock,			"Test Delta Clock"},
	{	"printdc",		"ldc",		listDeltaClockTask,		"Print Delta Clock Contents"},

	// Semaphores Tasks - 3
	{	"testsems",		"ts",		testSignals,			"Test Semaphores"},
	{	"signal1",		"s1",		signal1,				"Signal Semaphore 1"},
	{	"signal2",		"s2",		signal2,				"Signal Semaphore 2"},

	// Jurassic Park Tasks - 1
	{	"jurassic park","jp",		jurassicTask,			"Start Jurassic Park"},

	// Virtual Memory - 14
	{	"frame",		"dfm",		dumpFrame,				"Dump LC-3 memory frame"},
	{	"frametable",	"dft",		dumpFrameTable,			"Dump bit frame table"},
	{	"pagetable",	"pft",		dumpPageTable,			"Dump bit page table"},
	{	"memory",		"dm",		dumpLC3Mem,				"Dump LC-3 memory"},
	{	"page",			"dp",		dumpPageMemory,			"Dump swap page"},
	{	"virtual",		"dvm",		dumpVirtualMem,			"Dump virtual memory page"},
	{	"initmemory",	"im",		initMemory,				"Initialize virtual memory"},
	{	"root",			"rpt",		rootPageTable,			"Display root page table"},
	{	"user",			"upt",		userPageTable,			"Display user page table"},
	{	"touch",		"vma",		vmaccess,				"Access LC-3 memory location"},
	{	"stats",		"vms",		virtualMemStats,		"Output virtual memory stats"},
	{	"testvm",		"tvm",		vm_test,				"Test Virtual Memory"},
	{	"crawler",		"cra",		crawler,				"Execute crawler.hex"},
	{	"memtest",		"mem",		memtest,				"Execute memtest.hex"},

	// FAT - 20
	{	"testfat",		"tf",		fat_test,				"Execute file test"},
	{	"change",		"cd",		fat_cd,					"Change directory"},
	{	"copy",			"cf",		fat_copy,				"Copy file"},
	{	"define",		"df",		fat_define,				"Define file"},
	{	"delete",		"dl",		fat_del,				"Delete file"},
	{	"directory",	"dir",		fat_dir,				"List current directory"},
	{	"mount",		"md",		fat_mount,				"Mount disk"},
	{	"mkdir",		"mk",		fat_mkdir,				"Create directory"},
	{	"run",			"run",		fat_run,				"Execute LC-3 program"},
	{	"type",			"ty",		fat_type,				"Type file"},
	{	"unmount",		"um",		fat_unmount,			"Unmount disk"},
	{	"fat",			"ft",		fat_dfat,				"Display fat table"},
	{	"fileslots",	"fs",		fat_fileSlots,			"Display current open slots"},
	{	"sector",		"ds",		fat_dumpSector,			"Display disk sector"},
	{	"chkdsk",		"ck",		fat_chkdsk,				"Check disk"},
	{	"open",			"op",		fat_open,				"Open file test"	},
	{	"read",			"rd",		fat_read,				"Read file test"	},
	{	"write",		"wr",		fat_write,				"Write file test"	},
	{	"seek",			"sk",		fat_seek,				"Seek file test"	},
	{	"close",		"cl",		fat_close,				"Close file test"	}

};
