#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 200
#define SIZE_OF_CHAR_POINTER sizeof(char*)
char ** command_history;
int history_count = 0;
FILE * fp;

char** parse_command(char command[]);
void execute_command(char ** args);
int check_history(char command[]);
int execute_excp_command(char **args);
int read_bash_history();
void add_command_history(char command[], int write_file);