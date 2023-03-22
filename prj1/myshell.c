#include "myshell.h"


int main(){
    char command[MAX_COMMAND_LENGTH];
    char ** args;
    do {
        printf("CSE4100:P1-myshell>");
        fgets(command,MAX_COMMAND_LENGTH,stdin);
        args = parse_command(command);
        execute_command(args);

        free(args);
    }while (1);
    
}

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
    char path[1024];
    __pid_t pid;
    getcwd(path,sizeof(path));
    if(pid = fork() == 0){
        //child process
        execlp(path, args[0],NULL);
    }else{
        //parent process
        wait();
    }
}