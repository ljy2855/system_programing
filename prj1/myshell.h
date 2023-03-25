#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "csapp.h"

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 200
#define MAX_PIPELINE 10
#define SIZE_OF_CHAR_POINTER sizeof(char*)
#define SIZE_OF_CHAR_DOUBLE_POINTER sizeof(char**)

typedef struct JOB Job;
struct JOB{
    Job * prev;
    Job * next;
    int id;
    int status;
    pid_t pid;
    char *** args;

};
Job * first_job;
Job * last_job;
int curruent_job_id = 1;

void parse_command(char command[],int * pipe_commands_count, char ** args[]);
void execute_command(char ** args[], int pipe_count);
int execute_excp_command(char ** args);
int create_sub_process(int in, int out, char ** args);
int parse_bg_command(char ** args);
void create_bg_process(pid_t pid,char ** args[]);
void print_bg_process_create(pid_t pid, int id);