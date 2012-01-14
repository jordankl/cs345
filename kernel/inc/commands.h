#ifndef COMMANDS_H
#define COMMANDS_H
typedef struct								// command struct
{
	char* command;
	char* shortcut;
	int (*func)(int, char**);
	char* description;
} Command;

extern Command builtin_cmd_list[];
extern Command command_list[];
extern int number_commands;
extern int number_builtin_cmds;
#endif
