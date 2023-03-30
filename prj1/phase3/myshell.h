#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_PIPELINE 10
#define SIZE_OF_CHAR_POINTER sizeof(char*)
#define SIZE_OF_CHAR_DOUBLE_POINTER sizeof(char**)
#define process_running 0
#define process_stop 1
#define process_done 2
#define process_terminate 3

typedef struct JOB Job;
typedef void handler_t(int);
struct JOB{
    Job * prev;
    Job * next;
    int id;
    int status;
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];

};
Job * first_job;
Job * last_job;
pid_t current_pid;
pid_t root_pid;
char current_command[MAX_COMMAND_LENGTH];
int curruent_job_id = 1;
int saved_stdin;
int saved_stdout;
char **command_history;
int history_count = 0;
FILE *fp;
char project_path[200];

void parse_command(char command[], int *pipe_commands_count, char **args[]);
void execute_command(char command[],char ** args[], int pipe_count);
int execute_excp_command(char ** args);
int create_sub_process(int in, int out, char ** args);
int parse_bg_command(char ** args);
int process_background_command(char **args);
void create_bg_process(pid_t pid, char command[], int is_stop);
void print_bg_process_create(pid_t pid, int id);
void print_bg_process_state(Job * job,int status);
void remove_char(char * str,const char ch);
void remove_job_node(Job *job);
void terminate_process_handler(int sig);
void suspend_process_handler(int sig);
void resume_process_handler(int sig);
int read_bash_history();
void add_command_history(char command[], int write_file);
int replace_history_command(char command[]);

void Kill(pid_t pid, int signum);
ssize_t Sio_puts(char s[]);
void unix_error(char *msg);
void sio_error(char s[]);
ssize_t sio_puts(char s[]);
handler_t *Signal(int signum, handler_t *handler);
