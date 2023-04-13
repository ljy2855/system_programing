#include "myshell.h"

int main()
{
    char command[MAX_COMMAND_LENGTH];
    char **args[MAX_PIPELINE];
    int pipe_count;
    int i;
    read_bash_history(); // read before history

    do
    {
        pipe_count = 0;
        printf("CSE4100-MP-P1>");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        if (command[0] == '\n') // if enter, pass loop
            continue;
        command[strcspn(command, "\n")] = 0; // remove \n

        if (!replace_history_command(command)) // if can't replace history command, pass loop
            continue;
        parse_command(command, &pipe_count, args);
        if (*args[0] == NULL) // if command is just blank, pass loop
            continue;
        add_command_history(command, 1);

        execute_command(args, pipe_count);
        memset(command,0,MAX_COMMAND_LENGTH);
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
    len = strlen(command);
    int flag = 0; // flag store success replacing history command
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
        char **result = (char **)malloc(SIZE_OF_CHAR_POINTER); // allocate result to store argvs
        temp = strtok(pipe_command, " ");
        //store first command ex)ls, cd
        result[count] = temp;

        count++;
        while (temp != NULL)
        {
            //if next argv is null, break
            result = realloc(result, SIZE_OF_CHAR_POINTER * (count + 1));

        
            temp = strtok(NULL, " \'\"");

            result[count] = temp;

            count++;
        }
        // add color option
        if(!strcmp(result[0], "ls") || !strcmp(result[0], "grep")){ 
            result = realloc(result, SIZE_OF_CHAR_POINTER * (count + 1));
            result[count-1] = color_option;
            result[count] = NULL;
        }
        
        // store all argvs
        args[pipe_count++] = result;
       
        if (next_command != NULL)
            pipe_command = next_command + sizeof(char);

    } while (next_command != NULL);
     // store pipe counts'
    *pipe_commands_count = pipe_count;
}

void execute_command(char **args[], int pipe_count)
{
    int status; // store child process status
    pid_t pid;
    int in, fd[2];  // store stdin fd
    in = 0;
    int i = 0;
    int saved_stdout = dup(1);
    int saved_stdin = dup(0);

    for (i = 0; i < pipe_count - 1; ++i) // excute pipe command
    {
        pipe(fd); // open pipe to connect excute output with next input of excute

        create_sub_process(in, fd[1], args[i]);
        close(fd[1]); // close unused pipe
        in = fd[0]; // connect current output to next input
    }
    if (in != STDIN_FILENO) // if current input is not stdin
    {
        dup2(in, 0); // connect current input to stdin
    }
    if (i == 0)
    {
        if (execute_excp_command(args[i]))
            return;
    }

    if (pid = fork() == 0)
    {

         //child process
        if (execvp(args[i][0], args[i]) < 0)
        {
            // if excution failed, print error
            printf("%s: Command not found.\n", args[i][0]);
            exit(1); // return abort
        }
        close(0); // close stdin
        exit(0);
    }
    else
    {

        waitpid(pid, &status, 0);
        dup2(saved_stdin, 0); // recover stdin
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
            // if input is not stdin, connect input to stdin, close input
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if (out != STDOUT_FILENO)
        {
            // if output is not stdout, connect output to stdout, close output
            dup2(out, STDOUT_FILENO);
            close(out);
        }

        if (!execute_excp_command(args)) // check this is built in command
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
        waitpid(pid, &status, 0); // wait child process terminated

    return pid;
}


int execute_excp_command(char **args)
{
    int i = 0;
    if (strcmp(args[0], "cd") == 0)
    {
        // change directory 
        chdir(args[1]);
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0)
    {
         //free history 
        for (; i < history_count; i++)
        {
            free(command_history[i]);
        }
        free(command_history);
        exit(1);
    }
    else if (strcmp(args[0], "history") == 0)
    {
        // print all history
        for (; i < history_count; i++)
        {
            printf("%d\t%s\n", i + 1, command_history[i]);
        }

        return 1;
    }
    return 0;
}