#include "myshell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char ** args;
    read_bash_history();
    do
    {
        printf("CSE4100-MP-P1>");
        fgets(command,MAX_COMMAND_LENGTH,stdin);
        if(command[0] == '\n')
            continue;
        command[strcspn(command, "\n")] = 0;
        if(!check_history(command)){
            args = parse_command(command);
            execute_command(args);
            free(args);
        }

    } while (1);
}
int read_bash_history(){
    fp = fopen(".bash_history", "r");
    char command[MAX_COMMAND_LENGTH] = {0,};
    char *temp;

    while(!feof(fp)){
        fgets(command, MAX_COMMAND_LENGTH, fp);
        if (command[0] == '\n')
            continue;
        command[strcspn(command, "\n")] = 0;
        add_command_history(command, 0);
        //printf("%s", command_history[history_count - 1]);
    }
    fclose(fp);
}
void add_command_history(char command[],int write_file){
    history_count++;
    command_history = (char **)realloc(command_history, sizeof(char *) * history_count);
    command_history[history_count - 1] = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    strcpy(command_history[history_count - 1], command);
    if(write_file){
        
        fp = fopen(".bash_history", "a");
       
        fprintf(fp, "\n%s", command);
        fclose(fp);
    }
}

int check_history(char command[]){
    int i =0;
    int target = 0;
    if(strcmp(command,"history") == 0){
        for(;i<history_count;i++){
            printf("%d\t%s\n",i+1,command_history[i]);
        }

        add_command_history(command, 1);
        return 1;
    }else if(command[0] == '!'){
        if(command[1] == '!'){
            printf("%s\n",command_history[history_count-1]);
            target = history_count - 1;
        }else if(atoi(command+1) != 0){
            if(atoi(command+1) <= history_count){
                target = atoi(command+1) -1;
                printf("%s\n",command_history[target]);
                add_command_history(command_history[target], 1);
            }

            else
            {
                printf("out of index history\n");
                return 1;
            }
            
        }
        strcpy(command,command_history[target]);
        return 0;
    }
    add_command_history(command, 1);
    return 0;
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
    int status;
    __pid_t pid;

    if(execute_excp_command(args)){
        return;

    }
        else
        {
            if (fork() == 0)
            {
                // child process
                // printf("%s\n",command);
                if (execvp(args[0], args) < 0)
                {
                    printf("%s: Command not found.\n", args[0]);
                    exit(0);
                }
            }
            else
            {
                // parent process

                wait(&status);
                if (WEXITSTATUS(status))
                    exit(1);
            }
        }
}

int execute_excp_command(char **args)
{
        int i = 0;
        if (strcmp(args[0], "cd") == 0)
        {
            chdir(args[1]);
            return 1;
    }
    else if (strcmp(args[0], "exit") == 0)
    {
            for (; i < history_count;i++)
            {
                free(command_history[i]);
            }
            free(command_history);
            exit(1);
    }
    else if (strcmp(args[0], "history") == 0)
    {
        for (; i < history_count-1; i++)
        {
                printf("%d\t%s\n", i + 1, command_history[i]);
        }
        //strcpy(command_history[history_count++], command);
        return 1;
    }

    return 0;
}