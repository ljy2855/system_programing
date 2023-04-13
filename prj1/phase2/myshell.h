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

/**
 * @brief read before command history and store 
 * 
 * @return int 
 */
int read_bash_history();
/**
 * @brief add and store command history 
 * 
 * @param command 
 * @param write_file 
 */
void add_command_history(char command[], int write_file);
/**
 * @brief replace history command (!!, !#) to target command
 * 
 * @param command 
 * @return int 
 */
int replace_history_command(char command[]);
/**
 * @brief command parser get pipe counts, args
 * 
 * @param command 
 * @param pipe_commands_count 
 * @param args 
 */
void parse_command(char command[], int *pipe_commands_count, char **args[]);
/**
 * @brief execute command
 * 
 * @param args 
 * @param pipe_count 
 */
void execute_command(char ** args[], int pipe_count);
/**
 * @brief excute built in command ex) exit, cd, history
 * 
 * @param args 
 * @return int 
 */
int execute_excp_command(char ** args);
/**
 * @brief Create a sub process object excute pipe command
 * 
 * @param in 
 * @param out 
 * @param args 
 * @return int 
 */
int create_sub_process(int in, int out, char ** args);
