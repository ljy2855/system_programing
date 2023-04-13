#include "myshell.h"

int main()
{
    char command[MAX_COMMAND_LENGTH];
    char **args[MAX_PIPELINE];
    int pipe_count;
    int i;
    sigset_t mask, prev;
    saved_stdin = dup(0);
    saved_stdout = dup(1);
    root_pid = getpid();
    read_bash_history(); // read before history

    // set interrupt, terminal stop, child process signal handler
    Signal(SIGINT, terminate_current_process_handler);
    Signal(SIGTSTP, suspend_process_handler);
    Signal(SIGCHLD, terminate_process_handler);

    do
    {
        // block terminal stop signal in myshell
        sigemptyset(&mask);
        sigaddset(&mask, SIGTSTP);
        sigprocmask(SIG_BLOCK, &mask, &prev);

        pipe_count = 0;
        printf("CSE4100-MP-P1>");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        if (command[0] == '\n') // if enter, pass loop
            continue;
        command[strcspn(command, "\n")] = 0; // remove \n
        if (!replace_history_command(command))
            continue; // if can't replace history command, pass loop

        strcpy(current_command, command);
        parse_command(command, &pipe_count, args);
        if (*args[0] == NULL)
            continue; // if command is just blank, pass loop
        add_command_history(command, 1);

        sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock terminal stop signal
        execute_command(command, args, pipe_count);

        for (i = 0; i < pipe_count; i++)
            free(args[i]);
    } while (1);
}
int read_bash_history()
{
    // open bash_history
    getcwd(project_path, 300);
    strcat(project_path, "/.bash_history");
    fp = fopen(project_path, "a");
    fclose(fp);
    fp = fopen(project_path, "r");
    char command[MAX_COMMAND_LENGTH] = {
        0,
    };
    char *temp;

    // read all command history
    while (!feof(fp))
    {
        fgets(command, MAX_COMMAND_LENGTH, fp);
        if (command[0] == '\n' || command[0] == 0) // if command is blank , pass
            continue;
        command[strcspn(command, "\n")] = 0;
        add_command_history(command, 0);
    }
    fclose(fp);
}
void add_command_history(char command[], int write_file)
{
    // command is same with previous command, pass
    if (history_count)
        if (!strcmp(command, command_history[history_count - 1]))
            return;

    history_count++;
    command_history = (char **)realloc(command_history, sizeof(char *) * history_count);
    command_history[history_count - 1] = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    strcpy(command_history[history_count - 1], command);
    // if write flag is set, write it to file
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
    int flag = 0; // flag store success replacing history command
    len = strlen(command);
    for (cur = 0; cur < len - 1; cur++)
    {
        if (command[cur] == '!')
        {
            if (command[cur + 1] == '!') // if !! command match, replace !! to target command
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
                flag = 1;
            }
            else if (atoi(command + cur + 1)) // if !# command match, replace !# to target command
            {
                index = atoi(command + cur + 1);
                sprintf(atoi_str, "%d", index);
                if (index > history_count || index < 0)
                {
                    printf("-bash: !%s: event not found\n", atoi_str);
                    return 0;
                }

                // printf("%s",atoi_str);
                strcpy(temp, command_history[index - 1]);
                len += strlen(temp) - strlen(atoi_str) + 1;
                strcat(temp, command + cur + strlen(atoi_str) + 1);
                strcpy(command + cur, temp);
                flag = 1;
            }
            else if (command[cur + 1] == '0')
            {
                printf("-bash: !0: event not found\n");
                return 0;
            }
        }
    }
    if (flag)
        printf("%s\n", command);
    return 1;
}

void parse_command(char command[], int *pipe_commands_count, char **args[])
{

    char *temp;
    int count;
    int pipe_count = 0;
    char *pipe_command = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    char *next_command;
    char *color_option = (char *)malloc(sizeof(char) *20);
    strcpy(color_option, "--color=auto");
    strcpy(pipe_command, command);

    do
    {
        next_command = strstr(pipe_command, "|"); //check command has pipe character
        if (next_command != NULL)
        {
            next_command[0] = 0;
        }

        count = 0;
        char **result = (char **)malloc(SIZE_OF_CHAR_POINTER);
        temp = strtok(pipe_command, " ");

        result[count] = temp;

        count++;
        while (temp != NULL)
        {
            result = realloc(result, SIZE_OF_CHAR_POINTER * (count + 1));
            if (temp[0] == '\"')
            {
                char *next_quote_index = strstr(temp + 1, "\"");
                // strncpy(result[count], temp, next_quote_index - temp);

                result[count] = temp + 1;

                *next_quote_index = 0;
                temp = next_quote_index + 1;
                temp = strtok(NULL, " ");
                printf("%s\n", result[count]);
                count++;

                continue;
            }
            else if (temp[0] == '\'')
            {
                // char *next_quote_index = strstr(temp + 1, "\'");
                temp = strtok(NULL, "\'");
                // strncpy(result[count], temp, next_quote_index - temp);
                result[count] = temp;
                // strncpy(result[count], temp+1, next_quote_index- temp -1);
                temp = strtok(NULL, "\'");
                // remove_char(result[count],'\'');
                // printf("%s\n",result[count]);

                count++;

                continue;
            }
            temp = strtok(NULL, " \'\"");

            result[count] = temp;

            count++;
        }
        result = realloc(result, SIZE_OF_CHAR_POINTER * (count + 1));
        result[count-1] = color_option;
        result[count] = NULL;
        
        args[pipe_count++] = result;

        if (next_command != NULL)
            pipe_command = next_command + sizeof(char);

    } while (next_command != NULL);

    *pipe_commands_count = pipe_count;
}

int process_background_command(char **args)
{
    sigset_t mask, prev;
    if (strcmp(args[0], "jobs") == 0)
    {
        Job *job = first_job;
        while (job != NULL)
        {
            if (job->status == PROCESS_RUNNING)
                print_bg_process_state(job, PROCESS_RUNNING);
            else
                print_bg_process_state(job, PROCESS_STOP);
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
        if (args[1][0] != '%')
        {
            Sio_puts("kill: usage: kill <job>\n");
            return 1;
        }

        while (job != NULL)
        {
            if (job->id == atoi(args[1] + 1))
            {
                kill(job->pid, SIGKILL);
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
        pid_t pid;
        int status;
        if (args[1] == NULL)
        {
            Sio_puts("fg: usage: fg <job>\n");
            return 1;
        }
        if (args[1][0] != '%')
        {
            Sio_puts("fg: usage: fg <job>\n");
            return 1;
        }
        while (job != NULL)
        {
            if (job->id == atoi(args[1] + 1))
            {
                sigaddset(&mask, SIGTSTP);
                sigprocmask(SIG_UNBLOCK, &mask, &prev);
                Signal(SIGCONT, resume_process_handler);

                Sio_puts(job->command);
                Sio_puts("\n");
                current_pid = job->pid;
                kill(job->pid, SIGCONT);

                waitpid(job->pid, &status, WUNTRACED);
                if (WIFSTOPPED(status))
                    print_bg_process_state(job, PROCESS_STOP);

                if (WIFEXITED(status))
                    if (pid == job->pid)
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
        char output[MAX_COMMAND_LENGTH * 2];
        if (args[1] == NULL)
        {
            Sio_puts("bg: usage: bg <job>\n");
            return 1;
        }
        if (args[1][0] != '%')
        {
            Sio_puts("bg: usage: bg <job>\n");
            return 1;
        }
        while (job != NULL)
        {
            if (job->id == atoi(args[1] + 1))
            {
                strcat(job->command, " &");
                sprintf(output, "[%d] %s\n", job->id, job->command);
                current_pid = job->pid;
                Sio_puts(output);

                job->status = PROCESS_RUNNING;
                kill(job->pid, SIGCONT);

                return 1;
            }

            job = job->next;
        }
        Sio_puts("bg: no such job\n");
        return 1;
    }
    return 0;
}

int parse_bg_command(char **args)
{
    
    char *cur, *temp;
    cur = args[0];
    int count = 0;

    while (cur != NULL)
    {
        if ((temp = strchr(cur, '&')) != NULL)
        {
            if (cur[0] == '&')
                args[count] = NULL;
            else
                temp[0] = 0;

            return 1;
        }

        cur = args[++count];
    }
    return 0;
}

void execute_command(char command[], char **args[], int pipe_count)
{
    int status;
    pid_t pid;
    int in, fd[2];
    in = STDIN_FILENO;
    int i = 0;
    int is_bg_process = 0;
    if (process_background_command(args[0]))
        return;

    for (i = 0; i < pipe_count - 1; ++i)
    {
        pipe(fd);

        create_sub_process(in, fd[1], args[i]);
        close(fd[1]);
        in = fd[0];
    }
    if (in != STDIN_FILENO)
    {
        dup2(in, 0);
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
        current_pid = pid;

        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);

        if (is_bg_process)
        {
            create_bg_process(pid, command, 0);
        }

        if (!is_bg_process)
        {
            waitpid(pid, &status, WUNTRACED);
        }
    }
}

int create_sub_process(int in, int out, char **args)
{
    pid_t pid;
    int status;
    if ((pid = fork()) == 0)
    {
        if (in != STDIN_FILENO)
        {
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if (out != STDOUT_FILENO)
        {
            dup2(out, STDOUT_FILENO);
            close(out);
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
        waitpid(pid, &status, 0);

    return pid;
}

void create_bg_process(pid_t pid, char command[], int is_stop)
{
    Job *new_job = (Job *)malloc(sizeof(Job));
    setpgid(pid, pid);
    new_job->pid = pid;
    new_job->prev = NULL;
    new_job->next = NULL;
    new_job->id = curruent_job_id++;
    if (is_stop)
        new_job->status = PROCESS_STOP;
    else
        new_job->status = PROCESS_RUNNING;
    strcpy(new_job->command, command);
    remove_char(new_job->command, '&');

    if (first_job == NULL)
        first_job = new_job;

    if (last_job != NULL)
    {
        last_job->next = new_job;
        new_job->prev = last_job;
    }
    last_job = new_job;
    if (new_job->status == PROCESS_RUNNING)
        print_bg_process_create(new_job->pid, new_job->id);
    else
        print_bg_process_state(new_job, PROCESS_STOP);
}

void print_bg_process_create(pid_t pid, int id)
{
    char output[MAX_COMMAND_LENGTH] = "";
    sprintf(output, "[%d] %d\n", id, pid);
    Sio_puts(output);
}

void print_bg_process_state(Job *job, int status)
{
    char output[MAX_COMMAND_LENGTH * 2] = "";
    char state[20];
    switch (status)
    {
    case PROCESS_DONE:
        strcpy(state, "Done");
        break;

    case PROCESS_RUNNING:
        strcpy(state, "Running");
        break;
    case PROCESS_STOP:
        strcpy(state, "Stopped");
        break;
    case PROCESS_TERMINATE:
        strcpy(state, "Terminated");
        break;
    }
    sprintf(output, "[%d]\t%s\t\t %s\n", job->id, state, job->command);

    Sio_puts(output);
}

void remove_char(char *str, const char ch)
{
    int len = strlen(str) + 1;
    for (; *str != '\0'; str++, len--) // 종료 문자를 만날 때까지 반복
    {
        if (*str == ch) // ch와 같은 문자일 때
        {
            strncpy(str, str + 1, len);
            str--;
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
        for (; i < history_count; i++)
        {
            free(command_history[i]);
        }
        free(command_history);
        exit(0);
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

void remove_job_node(Job *job)
{
    if (job == first_job)
    {
        first_job = job->next;
    }
    if (job == last_job)
    {
        last_job = job->prev;
        
        if(last_job == NULL)
            curruent_job_id = 1;
        else
            curruent_job_id = last_job->id + 1;
    }
    if (job->next != NULL)
        job->next->prev = job->prev;
    if (job->prev != NULL)
        job->prev->next = job->next;

    free(job);
}

void terminate_process_handler(int sig)
{
    int status;
    pid_t pid;
    Job *cur = first_job;
    int flag = 0;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        while (cur != NULL)
        {
            if (cur->pid == pid)
            {
                if (WIFEXITED(status))
                {
                    Sio_puts("\n");
                    print_bg_process_state(cur, PROCESS_DONE);
                    remove_job_node(cur);
                }

                else if (WTERMSIG(status) == SIGKILL)
                {
                    Sio_puts("\n");
                    print_bg_process_state(cur, PROCESS_TERMINATE);
                    remove_job_node(cur);
                }

                if (cur->pid = PROCESS_STOP)
                {
                    flag = 1;
                }
                break;
            }
            cur = cur->next;
        }
        if (flag)
            break;
    }
}

void suspend_process_handler(int sig)
{

    if (current_pid == 0)
    {
        return;
    }

    Job *job = first_job;
    while (job != NULL)
    {
        if (job->pid == current_pid)
        {
            job->status = PROCESS_STOP;
            remove_char(job->command, '&');
            kill(current_pid, SIGSTOP);
            return;
        }

        job = job->next;
    }
    create_bg_process(current_pid, current_command, PROCESS_STOP);
}
void resume_process_handler(int sig)
{
    kill(current_pid, SIGCONT);
}

void terminate_current_process_handler(int sig)
{
    pid_t pid = current_pid;
    Job *job;
    job = first_job;
    if (pid != 0)
    {

        kill(pid, SIGKILL);

        while (job != NULL)
        {
            if (job->pid == pid)
            {
                remove_job_node(job);
            }

            job = job->next;
        }
    }
}

void Kill(pid_t pid, int signum)
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
        unix_error("kill error");
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
