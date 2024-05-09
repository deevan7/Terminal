#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>

// Define exit status codes
#define INVALID_VARIABLE -1
#define INVALID_VALUE -2
#define NON_EXISTING_VARIABLE -3
#define MAX_ENV_VAR_NUM_REACHED -4
#define INCOMPLETE_QUOTES -5

int wstatus;              // variable to store the wait status of a child process
int last_exit_status = 0; // variable to store the exit code of the recently executed command

// Array to store environment variables
char *environment_variables[29] = {"FOO", "", "USER", "Deevan", "PATH", "/usr/bin", NULL};

// Function to set environment variables
int set_env_variable(char *variable, char *val)
{
    int i = 0;
    last_exit_status = 0;
    while (environment_variables[i] != NULL)
    {
        if (strcmp(environment_variables[i], variable) == 0)
        {
            environment_variables[i + 1] = strdup(val);
            return 0;
        }
        i = i + 2;
    }
    if (i < 30)
    {
        environment_variables[i] = strdup(variable);
        environment_variables[i + 1] = strdup(val);
        return 0;
    }

    printf("flash :: Maximum number of environment variable reached\n");
    last_exit_status = MAX_ENV_VAR_NUM_REACHED;
    return MAX_ENV_VAR_NUM_REACHED;
}

// Function to set environment variables
int get_env_var(char *variable, char *value)
{
    int i = 0;
    last_exit_status = 0;
    while (environment_variables[i] != NULL)
    {
        if (strcmp(environment_variables[i], variable) == 0)
        {
            strcpy(value, environment_variables[i + 1]);
            return 0;
        }
        i = i + 2;
    }
    printf("flash:: The environment variable doesn't exist\n");
    last_exit_status = NON_EXISTING_VARIABLE;
    return NON_EXISTING_VARIABLE;
}

// function to parse the cmd args
int option_array(char *arr[], char *s, int start)
{
    int i = 0;
    int j = start;
    int k = 0;
    // Omitting the leading spaces
    while (s[j] == ' ')
    {
        j++;
    }
    char tmp[25] = "";
    // Iterate through the options string
    int squotes = 1;
    int dquotes = 1;
    while (s[j] != '\0')
    {
        // Split options by space
        if (s[j] == ' ')
        {
            tmp[k] = '\0';
            arr[i] = strdup(tmp);
            i++;
            strcpy(tmp, "");
            // test
            k = 0;
            j++;
            while (s[j] == ' ') // omitting spaces in between
            {
                j++;
            }
        }
        else if (s[j] == '\'')
        {
            squotes = -1 * squotes; // omitting the quotes
            j++;
        }
        else if (s[j] == '\"')
        {
            dquotes = -1 * dquotes;
            j++;
        }
        else
        {
            tmp[k++] = s[j];
            j++;
        }
    }

    if (squotes == -1 || dquotes == -1)
    { // checking if the quotes are complete
        last_exit_status = INCOMPLETE_QUOTES;
        printf("flash :: incomplete Quotes\n");
        return INCOMPLETE_QUOTES;
    }

    tmp[k] = '\0';
    if (strcmp(tmp, "")) // if not an empty string, storing the last argument
    {
        arr[i] = strdup(tmp);
        i++;
    }

    arr[i] = NULL;

    return 0;
}

// Function to parse the set command to get the variable and the value
int set_fn_parser(char *cmd, char *variable, char *val)
{
    int i = 4;
    while (cmd[i] == ' ')
    {
        i++;
    }
    int j = 0;
    while (cmd[i] != ' ' && cmd[i] != '=')
    {
        if (j == 16 || !(isupper(cmd[i])))
        {
            printf("flash:: Inavlid Variable name\n");
            last_exit_status = INVALID_VARIABLE;
            return INVALID_VARIABLE;
        }
        variable[j] = cmd[i];
        j++;
        i++;
    }
    variable[j] = '\0';

    while (cmd[i] == ' ')
    {
        i++;
    }

    if (cmd[i] != '=')
    {
        printf("flash:: Inavlid Variable name\n");
        last_exit_status = INVALID_VARIABLE;
        return INVALID_VARIABLE;
    }
    else
    {
        i++;
    }

    while (cmd[i] == ' ')
    {
        i++;
    }

    int quotes = 0;
    if (cmd[i] == '\"')
    {
        i++;
        quotes = 1;
    }

    j = 0;

    while (i < strlen(cmd) && cmd[i] != '\"')
    {
        if (j == 240)
        {
            printf("flash:: Inavlid Value\n");
            last_exit_status = INVALID_VALUE;
            return INVALID_VALUE;
        }

        val[j] = cmd[i];
        i++;
        j++;
    }
    val[j] = '\0';
    if (cmd[i] != '\"')
    {
        if (quotes)
        {
            printf("flash:: Inavlid Value\n");
            last_exit_status = INVALID_VALUE;
            return INVALID_VALUE;
        }
    }
    else
    {
        if (!quotes)
        {
            printf("flash:: Inavlid Value\n");
            last_exit_status = INVALID_VALUE;
            return INVALID_VALUE;
        }
    }

    return 0;
}

// Function to parse the get command to get the variable
int get_fn_parser(char *cmd, char *variable)
{
    int i = 4;
    while (cmd[i] == ' ')
    {
        i++;
    }
    int j = 0;
    if (cmd[i] == '?')
    {
        variable[0] = cmd[i];
        variable[1] = '\0';
        i++;
        while (cmd[i] != '\0')
        {
            if (cmd[i] != ' ')
            {
                last_exit_status = INVALID_VARIABLE;
                printf("flash:: Inavlid Variable name\n");
                return INVALID_VARIABLE;
            }
            i++;
        }
        return 0;
    }
    while (cmd[i] != ' ' && cmd[i] != '\0')
    {
        if (j == 16 || !(isupper(cmd[i])))
        {
            printf("flash:: Inavlid Variable name\n");
            last_exit_status = INVALID_VARIABLE;
            return INVALID_VARIABLE;
        }
        variable[j] = cmd[i];
        i++;
        j++;
    }
    variable[j] = '\0';

    while (cmd[i] != '\0')
    {
        if (cmd[i] != ' ')
        {
            printf("flash:: Inavlid Variable name\n");
            last_exit_status = INVALID_VARIABLE;
            return INVALID_VARIABLE;
        }
        i++;
    }
    return 0;
}

// Function to handle input/output redirection
int infile_outfile(char *cmd, char *infile, char *outfile, int *infile_fd, int *outfile_fd)
{
    int i = 0;
    int dquotes = 1;
    int squotes = 1;
    while (i < strlen(cmd))
    { // storing infile and outfile
        if (cmd[i] == '<')
        {
            if (squotes == -1 || dquotes == -1)
            {
                i++;
                continue;
            }
            i++;
            while (cmd[i] == ' ')
            {
                i++;
            }
            int j = 0;
            while (i < strlen(cmd) && cmd[i] != '<' && cmd[i] != '>')
            {
                infile[j] = cmd[i];
                j++;
                i++;
            }
            i--;
        }
        else if (cmd[i] == '>')
        {
            if (squotes == -1 || dquotes == -1)
            {
                i++;
                continue;
            }
            i++;
            while (cmd[i] == ' ')
            {
                i++;
            }
            int j = 0;
            while (i < strlen(cmd) && cmd[i] != '<' && cmd[i] != '>')
            {
                outfile[j] = cmd[i];
                j++;
                i++;
            }
            i--;
        }
        else if (cmd[i] == '\'')
        {
            squotes = -1 * squotes;
        }
        else if (cmd[i] == '\"')
        {
            dquotes = -1 * dquotes;
        }
        i++;
    }

    if (strcmp(infile, "") || strcmp(outfile, ""))
    { // reassigning the cmd with no infile and outflie
        i = 0;
        while (i < strlen(cmd))
        {
            if ((cmd[i] == '<') || (cmd[i] == '>'))
            {
                cmd[i] = '\0';
                break;
            }
            i++;
        }

        if (strcmp(infile, ""))
        {
            i = strlen(infile);
            while (infile[i - 1] == ' ')
            {
                infile[i - 1] = '\0';
                i--;
            }
        }
        if (strcmp(outfile, ""))
        {
            i = strlen(outfile);
            while (outfile[i - 1] == ' ')
            {
                outfile[i - 1] = '\0';
                i--;
            }
        }
    }

    if (strcmp(infile, ""))
    {
        if ((*infile_fd = open(infile, O_RDONLY)) == -1)
        {
            perror("flash:");
            last_exit_status = errno;
            return errno;
        }
    }

    if (strcmp(outfile, ""))
    {
        if ((*outfile_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        {
            perror("flash:");
            last_exit_status = errno;
            return errno;
        }
    }
    return 0;
}

// Function to execute commands
int cmd_execution(char cmd[], int start, int end)
{
    int is_pipe = 0;
    char generator[25] = "";
    char consumer[25] = "";
    char *gen_opt[20];
    char *con_opt[20];
    int pipe_index;
    int pipefd[2];
    pid_t cpid1, cpid2;
    char cmd1[50];
    int is_bg = 0; // Flag to check if the command should run in the background
    char gen_infile[100] = "";
    char gen_outfile[100] = "";
    int gen_infile_fd = 0;
    int gen_outfile_fd = 1;
    char con_infile[100] = "";
    char con_outfile[100] = "";
    int con_infile_fd = 0;
    int con_outfile_fd = 1;
    char variable[15] = "";
    char value[240] = "";
    int custom_fn = 0;
    int i = 0;
    while (start <= end)
    {
        cmd1[i] = cmd[start];
        i++;
        start++;
    }
    cmd1[i] = '\0';

    if (cmd1[strlen(cmd1) - 1] == '#')
    {
        is_bg = 1;
        // Removing '#' character from the command
        cmd1[strlen(cmd1) - 1] = '\0';
    }

    i = 0;
    int squotes = 1;
    int dquotes = 1;
    while (i < strlen(cmd1))
    {
        if (cmd1[i] == '|') // checking for pipe
        {
            if (squotes == -1 || dquotes == -1)
            { // not to confuse pipe with the arguments
                i++;
                continue;
            }
            pipe_index = i;
            is_pipe = 1;
            break;
        }
        else if (cmd[i] == '\'')
        {
            squotes = -1 * squotes;
        }
        else if (cmd[i] == '\"')
        {
            dquotes = -1 * dquotes;
        }
        i++;
    }

    if (!is_pipe)
    {
        pipe_index = i;
    }

    i = 0;
    while (i < pipe_index)
    {
        generator[i] = cmd1[i];
        i++;
    }
    generator[i] = '\0';

    int ec = 0;

    if ((ec = infile_outfile(generator, gen_infile, gen_outfile, &gen_infile_fd, &gen_outfile_fd) != 0))
    {
        return 0;
    }

    if (strcmp(generator, "exit") == 0)
    {
        return 1;
    }

    // Parse command for set and get commands
    if (strncmp(generator, "set ", 4) == 0)
    {
        int ec = 0;
        if ((ec = set_fn_parser(generator, variable, value)) == 0)
        {
            if (set_env_variable(variable, value) != 0)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (strncmp(generator, "get ", 4) == 0)
    {
        custom_fn = 1;
        if (get_fn_parser(generator, variable) != 0)
        {
            return 0;
        }
        if (strcmp(variable, "?") == 0)
        {
            printf("%d\n", last_exit_status);
            last_exit_status = 0;
        }
        else
        {

            if (get_env_var(variable, value) == 0)
            {
                printf("%s\n", value);
            }
            else
            {
                return 0;
            }
        }
    }

    if (is_pipe)
    {
        if (pipe(pipefd) == -1)
        {
            return errno;
        }

        int j = 0;
        i = pipe_index + 1;

        while (cmd[i] == ' ')
        {
            i++;
        }
        while (i < strlen(cmd1))
        {
            consumer[j] = cmd1[i];
            i++;
            j++;
        }
        consumer[j] = '\0';

        if ((ec = infile_outfile(consumer, con_infile, con_outfile, &con_infile_fd, &con_outfile_fd)) != 0)
        {
            return 0;
        }

        if (strcmp(consumer, "exit") == 0)
        {
            return 1;
        }

        if (strncmp(consumer, "set ", 4) == 0)
        {
            custom_fn = 1;

            int ec = 0;
            if ((ec = set_fn_parser(consumer, variable, value)) == 0)
            {
                if (set_env_variable(variable, value) != 0)
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else if (strncmp(consumer, "get ", 4) == 0)
        {

            if (get_fn_parser(consumer, variable) != 0)
            {
                return 0;
            }
            if (strcmp(variable, "?") == 0)
            {
                printf("%d\n", last_exit_status);
                last_exit_status = 0;
            }
            else
            {

                if (get_env_var(variable, value) == 0)
                {
                    printf("%s\n", value);
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    if (option_array(gen_opt, generator, 0) != 0)
    {
        return 0;
    }
    if (is_pipe)
    {
        if (option_array(con_opt, consumer, 0) != 0)
        {
            return 0;
        }
    }

    if ((cpid1 = fork()) == 0)
    {
        dup2(gen_infile_fd, 0);

        if (is_pipe)
        {
            /* Child reads from pipe */
            close(pipefd[0]); /* Close unused read end */
            dup2(pipefd[1], 1);
            close(pipefd[1]);
        }
        if (strcmp(gen_outfile, ""))
        {
            dup2(gen_outfile_fd, 1);
        }

        if (execvp(gen_opt[0], gen_opt) == -1)
        {
            last_exit_status = errno;
            return errno;
        }
    }
    else if (cpid1 == -1)
    {
        return errno;
    }
    else
    {
        if (is_pipe)
        {

            if ((cpid2 = fork()) == 0)
            {
                dup2(con_outfile_fd, 1);
                close(pipefd[1]);
                dup2(pipefd[0], 0);
                close(pipefd[0]);
                if (strcmp(con_infile, ""))
                {
                    dup2(con_infile_fd, 0);
                }
                if (execvp(con_opt[0], con_opt) == -1)
                {
                    last_exit_status = errno;
                    return errno;
                }
            }
            else if (cpid2 == -1)
            {
                return errno;
            }
            else
            {
                // Parent process
                close(pipefd[0]);
                close(pipefd[1]);
                if (is_bg == 0)
                {
                    waitpid(cpid1, NULL, 0); // Wait for first child process
                    waitpid(cpid2, &wstatus, 0);
                    if (WIFEXITED(wstatus))
                    {
                        last_exit_status = WEXITSTATUS(wstatus);

                    } // Wait for second child process
                }
            }
        }

        else
        {
            // Parent process without pipe
            if (is_bg == 0)
            {
                waitpid(cpid1, &wstatus, 0);
                if (WIFEXITED(wstatus))
                {
                    last_exit_status = WEXITSTATUS(wstatus);
                }
            } // Wait for child process
        }
    }
    if (custom_fn)
    {
        last_exit_status = 0; // if the previous command was a custom fn, set the exit status to 0
    }
    return 0;
}

int main()
{

    while (1)
    {
        char cmd[200] = "";
        printf("flash$ ");
        scanf("%199[^\n]", cmd);
        getchar();
        if (strcmp(cmd, "") == 0)
        {
            continue;
        }
        int start = 0;
        int i = 0;
        int ec = 0;
        int squotes = 1;
        int dquotes = 1;
        while (i < strlen(cmd))
        {
            if (cmd[i] == ',')
            {
                if (squotes == -1 || dquotes == -1)
                { // not to confuse sequence with the arguments
                    i++;
                    continue;
                }
                if ((ec = cmd_execution(cmd, start, i - 1)) == 1)
                {
                    return 0;
                }
                i++;
                start = i;
            }
            else
            {
                if (cmd[i] == '\'')
                {
                    squotes = -1 * squotes;
                }
                else if (cmd[i] == '\"')
                {
                    dquotes = -1 * dquotes;
                }
                i++;
            }
        }
        if ((ec = cmd_execution(cmd, start, strlen(cmd) - 1)) == 1)
        {
            return 0;
        }
        else if (ec != 0)
        {
            return ec;
        }
    }
    return 0;
}
