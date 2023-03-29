#include "myshell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char **args[MAX_PIPELINE];
    int pipe_count;
    int i;
    read_bash_history();
    
    do
    {
        pipe_count = 0;
        printf("CSE4100-MP-P1>");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        if(command[0] == '\n') // if enter, pass loop
            continue;
        command[strcspn(command, "\n")] = 0; // remove \n

        if (!replace_history_command(command))
            continue;
        //printf("%s\n",command);
        add_command_history(command,1);
        parse_command(command,&pipe_count, args);
        execute_command(args,pipe_count);


        for(i = 0 ; i < pipe_count; i++)
            //printf("%s\n",args[i][0]);
            free(args[i]);


    } while (1);
}
int read_bash_history()
{
    fp = fopen(".bash_history", "a");
    fclose(fp);
    fp = fopen(".bash_history", "r");
    char command[MAX_COMMAND_LENGTH] = {
        0,
    };
    char *temp;

    while (!feof(fp))
    {
        fgets(command, MAX_COMMAND_LENGTH, fp);
        if (command[0] == '\n' || command[0] == 0)
            continue;
        command[strcspn(command, "\n")] = 0;
        add_command_history(command, 0);
    }
    fclose(fp);
}
void add_command_history(char command[], int write_file)
{
    if (history_count)
        if (!strcmp(command, command_history[history_count - 1]))
            return;

    history_count++;
    command_history = (char **)realloc(command_history, sizeof(char *) * history_count);
    command_history[history_count - 1] = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    strcpy(command_history[history_count - 1], command);
    if (write_file)
    {

        fp = fopen(".bash_history", "a");

        fprintf(fp, "\n%s", command);
        fclose(fp);
    }
}
int replace_history_command(char command[])
{
    int cur;
    int index;
    int len;
    char temp[MAX_COMMAND_LENGTH];
    char atoi_str[100];
    len = strlen(command);
    for (cur = 0; cur < len - 1; cur++)
    {
        if (command[cur] == '!')
        {
            if (command[cur + 1] == '!')
            {
                if (history_count == 0)
                {
                    printf("-bash: !!: event not found\n");
                    return 0;
                }

                strcpy(temp, command_history[history_count - 1]);
                len += strlen(temp) - 2;
                if (strlen(command + cur + 2))
                    strcat(temp, command + cur + 2);
                strcpy(command + cur, temp);
            }
            else if (atoi(command + cur + 1))
            {
                index = atoi(command + cur + 1);
                sprintf(atoi_str, "%d", index);
                if (index > history_count)
                {
                    printf("-bash: !%s: event not found\n", atoi_str);
                    return 0;
                }

                // printf("%s",atoi_str);
                strcpy(temp, command_history[index - 1]);
                len += strlen(temp) - strlen(atoi_str) + 1;
                strcat(temp, command + cur + strlen(atoi_str) + 1);
                strcpy(command + cur, temp);
            }
        }
    }
    return 1;
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

void execute_command(char ** args[], int pipe_count){
    int status;
    pid_t pid;
    int in, fd [2];
    in = 0;
    int i  = 0;
    int saved_stdout = dup(1);
    int saved_stdin = dup(0);
    
  
    for (i = 0 ; i < pipe_count-1 ; ++i){
        pipe(fd);

        create_sub_process(in,fd[1],args[i]);
        close(fd[1]);
        in = fd[0];
        
    }
    if (in != 0){
        dup2 (in, 0);
    }
    if(i == 0){
        if (execute_excp_command(args[i]))
            return;
    }
        
    if(pid = fork() == 0 ){
        close(in);
        if (execvp(args[i][0], args[i]) < 0)
        {
            printf("%s: Command not found.\n", args[i][0]);
            exit(0);
        }
        close(0);
        

    }else{
        
        waitpid(pid, &status, 0);
        dup2(saved_stdin, 0);
    }
}

int create_sub_process(int in, int out, char ** args){
    pid_t pid;
    int status;
    if ((pid = fork ()) == 0)
    {
      if (in != STDIN_FILENO)
        {
          dup2 (in, STDIN_FILENO);
          close (in);
        }

      if (out != STDOUT_FILENO)
        {
            dup2 (out, STDOUT_FILENO);
            close (out);
        }
        if(!execute_excp_command(args)){
            if(execvp (args[0], args)<0){
                printf("%s: Command not found.\n", args[0]);
                exit(1);
            }
                }
            

       exit(0);
    }
    else
        waitpid(pid,&status,0);
    
    return pid;
}

void execute_pipeline_command(char** args){

}

int 
execute_excp_command(char ** args){
    int i = 0;
    if(strcmp(args[0],"cd") == 0){
        chdir(args[1]);
        return 1;
    }
    else if(strcmp(args[0],"exit") == 0){
        for (; i < history_count;i++)
        {
            free(command_history[i]);
        }
        free(command_history);
        exit(1);
    }else if (strcmp(args[0], "history") == 0)
    {

        for (; i < history_count; i++)
        {
                printf("%d\t%s\n", i + 1, command_history[i]);
        }
        
        return 1;
    }
    return 0;
}