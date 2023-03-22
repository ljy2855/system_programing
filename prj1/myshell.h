#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<string.h>

#define MAX_ARGS 100
#define MAX_COMMAND_LENGTH 200
#define SIZE_OF_CHAR_POINTER sizeof(char*)

char** parse_command(char command[]);
void execute_command(char ** args);