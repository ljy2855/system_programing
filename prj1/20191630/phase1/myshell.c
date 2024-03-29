#include "myshell.h"

int main()
{
    char command[MAX_COMMAND_LENGTH];
    char **args;

    read_bash_history(); // read before history
    do
    {
        printf("CSE4100-MP-P1>");
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        if (command[0] == '\n') // if enter, pass loop
            continue;
        command[strcspn(command, "\n")] = 0; // remove \n
        if (!replace_history_command(command)) // if can't replace history command, pass loop
            continue;
        args = parse_command(command);
        if (*args == NULL) // if command is just blank, pass loop
            continue;
        add_command_history(command, 1);
        execute_command(args);
        free(args);

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

char **parse_command(char command[])
{
    char **result;
    char *temp;
    int count = 0;
    char temp_command[MAX_COMMAND_LENGTH];
    strcpy(temp_command, command);
    result = (char **)malloc(SIZE_OF_CHAR_POINTER); // allocate result to store argvs
    temp = strtok(temp_command, " ");
    result[count] = temp; //store first command ex)ls, cd

    count++;
    while (temp != NULL)
    {
        //if next argv is null, break
        result = realloc(result, SIZE_OF_CHAR_POINTER * (count + 1));
        temp = strtok(NULL, " ");
        result[count] = temp;

        count++;
    }

    return result;
}

void execute_command(char **args)
{
    int status; // store child process status
    __pid_t pid;

    if (execute_excp_command(args))
    {
        return;
    }
    else
    {
        if (fork() == 0)
        {
            // child process
            if (execvp(args[0], args) < 0)
            {
                // if execution failed, print error
                printf("%s: Command not found.\n", args[0]);
                exit(0);
            }
        }
        else
        {
            // parent process

            wait(&status);
        }
    }
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