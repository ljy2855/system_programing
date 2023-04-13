#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 8192
#define MAX_PIPELINE 10
#define SIZE_OF_CHAR_POINTER sizeof(char*)
#define SIZE_OF_CHAR_DOUBLE_POINTER sizeof(char**)
#define PROCESS_RUNNING 0
#define PROCESS_STOP 1
#define PROCESS_DONE 2
#define PROCESS_TERMINATE 3

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
volatile sig_atomic_t current_pid;
pid_t root_pid;
char current_command[MAX_COMMAND_LENGTH];
volatile sig_atomic_t curruent_job_id = 1;
int saved_stdin;
int saved_stdout;
char **command_history;
int history_count = 0;
FILE *fp;
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
 * @param command 
 * @param args 
 * @param pipe_count 
 */
void execute_command(char command[],char ** args[], int pipe_count);
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
/**
 * @brief check background command
 * 
 * @param args 
 * @return int 
 */
int parse_bg_command(char ** args);
/**
 * @brief excute jobs command ex) kill, bg, fg
 * 
 * @param args 
 * @return int 
 */
int process_background_command(char **args);
/**
 * @brief Create a bg process object, and store in jobs list
 * 
 * @param pid 
 * @param command 
 * @param is_stop 
 */
void create_bg_process(pid_t pid, char command[], int is_stop);
/**
 * @brief print bg process state when created
 * 
 * @param pid 
 * @param id 
 */
void print_bg_process_create(pid_t pid, int id);
/**
 * @brief print each bg process state
 * 
 * @param job 
 * @param status 
 */
void print_bg_process_state(Job * job,int status);
/**
 * @brief remove target character in string
 * 
 * @param str 
 * @param ch 
 */
void remove_char(char * str,const char ch);
/**
 * @brief remove target job in job list
 * 
 * @param job 
 */
void remove_job_node(Job *job);
/**
 * @brief signal handler that reaping process and remove job 
 * 
 * @param sig 
 */
void terminate_process_handler(int sig);
/**
 * @brief signal handler that suspend process state
 * 
 * @param sig 
 */
void suspend_process_handler(int sig);
/**
 * @brief signal handler that restart process state
 * 
 * @param sig 
 */
void resume_process_handler(int sig);
/**
 * @brief signal handler that terminal interrupt (control c) kill current process
 * 
 * @param sig 
 */
void terminate_current_process_handler(int sig);


//csapp.h 
void Kill(pid_t pid, int signum);
ssize_t Sio_puts(char s[]);
void unix_error(char *msg);
void sio_error(char s[]);
ssize_t sio_puts(char s[]);
handler_t *Signal(int signum, handler_t *handler);
