#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 8192
#define MAX_PIPELINE 10
#define SIZE_OF_CHAR_POINTER sizeof(char*)
#define SIZE_OF_CHAR_DOUBLE_POINTER sizeof(char**)
char ** command_history;
int history_count = 0;
FILE * fp;
char project_path[200];
void parse_command(char command[],int * pipe_commands_count, char ** args[]);
void execute_command(char ** args[], int pipe_count);
int execute_excp_command(char ** args);
int create_sub_process(int in, int out, char ** args);
int read_bash_history();
void add_command_history(char command[], int write_file);
int replace_history_command(char command[]);