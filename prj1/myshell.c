#include "myshell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char **args[MAX_PIPELINE];
    int pipe_count;
    int i;
    do {
        pipe_count = 0;
        printf("CSE4100:P1-myshell>");
        fgets(command,MAX_COMMAND_LENGTH,stdin);
        if(command[0] == '\n') // if enter, pass loop
            continue;
        command[strcspn(command, "\n")] = 0; // remove \n 
        parse_command(command,&pipe_count, args);
        execute_command(args);

        for(i = 0 ; i < pipe_count; i++)
            printf("%s\n",args[i][0]);
            // free(args[i]);
    }while (1);
    
}

void parse_command(char command[],int * pipe_commands_count, char ** args[]){

    char * temp;
    int count;
    int pipe_count = 0;
    char * pipe_command = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    char * next_command;
    strcpy(pipe_command,command);
    
    do{
 
        next_command = strstr(pipe_command,"|");
        if(next_command != NULL){
            next_command[0] = 0;
        }

        
        count = 0;
        char ** result = (char**)malloc(SIZE_OF_CHAR_POINTER);
        temp = strtok(pipe_command," ");
    
        result[count] = temp;

        count++;
        while(temp != NULL){
            result = realloc(result,SIZE_OF_CHAR_POINTER * (count+1));
            temp = strtok(NULL," ");
   
            result[count] = temp;
            
            count++;

        }
        args[pipe_count++] = result;
        
        if(next_command != NULL)
            pipe_command = next_command+sizeof(char);
        

   
    }   
    while(next_command != NULL);

    *pipe_commands_count = pipe_count;

}

void execute_command(char ** args[]){
    int status;
    __pid_t pid;

    execute_excp_command(args[0]);
   
    if(fork() == 0){
        //child process
        //printf("%s\n",command);
        if(execvp(args[0][0], args[0]) < 0 ){
            printf("%s: Command not found.\n", args[0][0]);
            exit(0);
        }

    }
    else
    {
        //parent process
        
        wait(&status);
        // if (WIFEXITED(status)) 
        //     exit(1);
    
    }
  
}

void execute_pipeline_command(char** args){

}

void execute_excp_command(char ** args){
    if(strcmp(args[0],"cd") == 0){
        chdir(args[1]);
    }
    else if(strcmp(args[0],"exit") == 0){
        exit(1);
    }
}