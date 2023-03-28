#include "myshell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char ** args;
    do {
        printf("CSE4100:P1-myshell>");
        fgets(command,MAX_COMMAND_LENGTH,stdin);
        if(command[0] == '\n')
            continue;
        command[strcspn(command, "\n")] = 0;
        args = parse_command(command);
        execute_command(args);

        free(args);
    }while (1);
    
}

// TODO
// build in command ex) quit,exit

char** parse_command(char command[]){
    char** result;
    char * temp;
    int count = 0;

    result = (char**)malloc(SIZE_OF_CHAR_POINTER);
    temp = strtok(command," ");
    result[count] = temp;

    count++;
    while(temp != NULL){
        result = realloc(result,SIZE_OF_CHAR_POINTER * (count+1));
        temp = strtok(NULL," ");
        result[count] = temp;
        // strcpy(result[count],temp);
        count++;

    }


    return result;
}

void execute_command(char ** args){
    int status;
    __pid_t pid;

    if(strcmp(args[0],"cd") == 0){
        chdir(args[1]);
    }else if(strcmp(args[0],"exit") == 0){
        exit(EXIT_SUCCESS);
    }
    else{
        if(fork() == 0){
            //child process
            //printf("%s\n",command);
            if(execvp(args[0], args) < 0 ){
                printf("%s: Command not found.\n", args[0]);
                exit(0);
            }

        }
        else
        {
            //parent process
            
            wait(&status);
            if (WEXITSTATUS(status)) 
                exit(1);
        
        }
    }
}