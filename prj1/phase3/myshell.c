#include "myshell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char **args[MAX_PIPELINE];
    int pipe_count;
    int i;
    saved_stdin = dup(0);
    saved_stdout = dup(1);
    root_pid = getpid();
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
        add_command_history(command, 1);
        strcpy(current_command, command);
        parse_command(command, &pipe_count, args);
        execute_command(command,args,pipe_count);
 

        for(i = 0 ; i < pipe_count; i++)
            //printf("%s\n",args[i][0]);
            free(args[i]);
    } while (1);
}
int read_bash_history()
{
    getcwd(project_path, 300);
    strcat(project_path, "/.bash_history");
    fp = fopen(project_path, "a");
    fclose(fp);
    fp = fopen(project_path, "r");
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

        fp = fopen(project_path, "a");

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

int process_background_command(char ** args){
    if (strcmp(args[0], "jobs") == 0)
    {
        Job *job = first_job;
        while (job != NULL)
        {
            if (job->status == process_running)
                print_bg_process_state(job, process_running);
            else
                print_bg_process_state(job, process_stop);
            job = job->next;
        }
        return 1;
    }
    else if (strcmp(args[0], "kill") == 0)
    {
        Job *job = first_job;
        if (args[1] == NULL)
        {
            Sio_puts("kill: usage: kill <job>\n");
            return 1;
        }

        while (job != NULL)
        {
            if (job->id == atoi(args[1]))
            {
                Kill(job->pid, SIGKILL);
                return 1;
            }

            job = job->next;
        }
        Sio_puts("kill: no such job\n");
        return 1;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        Job *job = first_job;
        int status;
        if (args[1] == NULL)
        {
            Sio_puts("fg: usage: fg <job>\n");
            return 1;
        }
        while (job != NULL)
        {
            if (job->id == atoi(args[1]))
            {
                Signal(SIGCONT, resume_process_handler);
                Signal(SIGTSTP, suspend_process_handler);
                Sio_puts(job->command);
                Sio_puts("\n");
                Kill(job->pid, SIGCONT);
                waitpid(job->pid, &status, WUNTRACED);
                if (WIFEXITED(status))
                    remove_job_node(job);
                return 1;
            }

            job = job->next;
        }
        Sio_puts("fg: no such job\n");
        return 1;
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        Job *job = first_job;
        int status;
        if (args[1] == NULL)
        {
            Sio_puts("bg: usage: bg <job>\n");
            return 1;
        }
        while (job != NULL)
        {
            if (job->id == atoi(args[1]))
            {
                Signal(SIGCHLD, terminate_process_handler);
                current_pid = job->pid;
                Sio_puts(job->command);
                Sio_puts("\n");
                job->status = process_running;
                Kill(job->pid, SIGCONT);

                return 1;
            }

            job = job->next;
        }
        Sio_puts("bg: no such job\n");
        return 1;
    }
    return 0;
}

int parse_bg_command(char ** args){

    char * cur, *temp;
    cur = args[0];
    int count = 0;
    
    while(cur != NULL){
        if((temp = strchr(cur,'&')) != NULL){
            if(cur[0] == '&')
                args[count] = NULL;
            else
                temp[0] = 0;

            return 1;
        }
            
        cur = args[++count];
    }
    return 0;
}

void execute_command(char command[],char ** args[], int pipe_count){
    int status;
    pid_t pid;
    int in, fd [2];
    in = STDIN_FILENO;
    int i  = 0;
    int is_bg_process = 0;
    if(process_background_command(args[0]))
        return;

    Signal(SIGTSTP, suspend_process_handler);


    for (i = 0 ; i < pipe_count-1 ; ++i){
        pipe(fd);
      
        create_sub_process(in,fd[1],args[i]);
        close(fd[1]);
        in = fd[0];
        
        }
    if (in != STDIN_FILENO){
        dup2 (in, 0);
    }
    is_bg_process = parse_bg_command(args[i]);
    if (execute_excp_command(args[i]))
        return;

    if ((pid = fork()) == 0)
    {

        if (execvp(args[i][0], args[i]) < 0)
        {
            printf("%s: Command not found.\n", args[i][0]);
            exit(1);
        }
        
        close(0);
        exit(1);
    }
    else
    {
        //setpgid(pid,0);
        current_pid = pid;
        
    

        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout,STDOUT_FILENO);
        

        if(is_bg_process){
            create_bg_process(pid, command,0);
            Signal(SIGCHLD, (void *)terminate_process_handler);
            
        }
            
        if(!is_bg_process){
            Signal(SIGCHLD, (void *)terminate_process_handler);
            waitpid(pid, &status, WUNTRACED);
        }
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

        if (!execute_excp_command(args))
        {
            if (execvp(args[0], args) < 0)
            {
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

void create_bg_process(pid_t pid,char command[],int is_stop){
    Job * new_job = (Job *)malloc(sizeof(Job));
    
    new_job->pid = pid;
    new_job->prev = NULL;
    new_job->next = NULL;
    new_job->id = curruent_job_id++;
    if (is_stop)
        new_job->status = process_stop;
    else
        new_job->status = process_running;
    strcpy(new_job->command, command);
    remove_char(new_job->command,'&');



    if(first_job == NULL)
        first_job = new_job;
    

    if(last_job!= NULL){
        last_job->next = new_job;
        new_job->prev = last_job;
    }
    last_job = new_job;

    print_bg_process_create(new_job->pid,new_job->id);

}

void print_bg_process_create(pid_t pid, int id){
    char output[MAX_COMMAND_LENGTH] = "";
    sprintf(output,"[%d] %d\n",id,pid);
    Sio_puts(output);
}

void print_bg_process_state(Job * job,int status){
    char output[MAX_COMMAND_LENGTH] = "";
    char state[10];
    switch (status)
    {
    case process_done:
        strcpy(state, "Done");
        break;

    case process_running:
        strcpy(state, "Running");
        break;
    case process_stop:
        strcpy(state, "Stopped");
        break;
    case process_terminate:
        strcpy(state, "Terminated");
        break;
    }
   
    if (last_job == job)
    {
        sprintf(output, "[%d]+\t%s\t\t\t %s\n", job->id, state, job->command);
    }
    else if (last_job->prev == job)
    {
        sprintf(output, "[%d]-\t%s\t\t\t %s\n", job->id, state, job->command);
    }
    else
    {
        sprintf(output, "[%d]\t%s\t\t\t %s\n", job->id, state, job->command);
    }
    
    
    
    
    Sio_puts(output);
}

void remove_char(char * str,const char ch){
    int len = strlen(str) + 1;
    for (; *str != '\0'; str++,len--)//종료 문자를 만날 때까지 반복
    {
        if (*str == ch)//ch와 같은 문자일 때
        {
            strncpy(str, str+1, len);
            str--;            
        }
    }
}


int execute_excp_command(char ** args){
    int i = 0;
    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
        return 1;
    }
    else if(strcmp(args[0],"exit") == 0){
        for (; i < history_count; i++)
        {
            free(command_history[i]);
        }
        free(command_history);
        exit(1);
    }
    
    else if (strcmp(args[0], "history") == 0)
    {

        for (; i < history_count; i++)
        {
            printf("%d\t%s\n", i + 1, command_history[i]);
        }

        return 1;
    }

        return 0;
}

void remove_job_node(Job * job){
    if(job == first_job){
        first_job = job->next;
        
    }
    if(job == last_job){
        last_job = job->prev;
    }
    if (job->next != NULL)
        job->next->prev = job->prev;
    if (job->prev != NULL)
        job->prev->next = job->next;

    
    free(job);
}

void terminate_process_handler(int sig){
    int status;
    pid_t pid;
    Job *cur = first_job;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        
        while (cur != NULL)
        {
            if (cur->pid == pid)
            {
                if(WIFEXITED(status))
                    print_bg_process_state(cur, process_done);
                else
                {
                    Sio_puts("\n");
                    print_bg_process_state(cur, process_terminate);
                }
                    
                remove_job_node(cur);
                return;
            }
            cur = cur->next;
        }
    }
}

void suspend_process_handler(int sig){
    close(0);
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    //Kill(current_pid, SIGTSTP);
    //Kill(root_pid, SIGCONT);
    //Kill(current_pid, SIGCONT);
    
    Job *job = first_job;
    while (job != NULL)
    {
        if(job->pid == current_pid){
            job->status = process_stop;
            return;
        }

        job = job->next;
    }
    create_bg_process(current_pid, current_command, process_stop);

    //signal(SIGCHLD, (void *)terminate_process_handler);
    
}
void resume_process_handler(int sig){
    pid_t pgid = __getpgid(current_pid);
    Kill(pgid,SIGCONT);
    kill(current_pid,SIGCONT);
}

void Kill(pid_t pid, int signum)
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
        unix_error("Kill error");
}

ssize_t Sio_puts(char s[])
{
    ssize_t n;

    if ((n = sio_puts(s)) < 0)
        sio_error("Sio_puts error");
    return n;
}

void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
/* $end unixerror */

void sio_error(char s[]) /* Put error message and exit */
{
    sio_puts(s);
    _exit(1); // line:csapp:sioexit
}
/* $end siopublic */

ssize_t sio_puts(char s[]) /* Put string */
{
    return write(STDOUT_FILENO, s, strlen(s)); // line:csapp:siostrlen
}

handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}