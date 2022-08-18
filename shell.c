#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_TOKENS 128
#define DELE " \t\n "

int isBatchMode = 0;

struct command_struc
{
    char *name;
    char **arglist;
    int arglen;
    int isToBeExecuted;
    int isInputRedirec;
    int isOutputRedirec;
    int input;
    int output;
};
struct pipe_struc
{
    int ispipe;
    int readend;
    int writeend;
    int isLast;
};

typedef void (*command_func_ptr)(struct command_struc *, char **, int);

void execute_command(struct command_struc *, struct pipe_struc *);
char *supportedCommands[] = {
    "checkcpupercentage",
    "checkresidentmemory",
    "listFiles",
    "sortFile",
    "executeCommands",
    "grep",
    "cat",
    "exit",
    NULL};

int form_arguments(char **command, int len)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (command[i] != NULL)
        {
            char *tmp = command[i];
            command[i] = command[j];
            command[j] = tmp;
            j++;
        }
    }
    command[j] = NULL;
    return j;
}
int inputredirection(char **command, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (command[i] != NULL && strcmp(command[i], "<") == 0)
        {
            return i + 1;
        }
    }
    return -1;
}
int outputredirection(char **command, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (command[i] != NULL && strcmp(command[i], ">") == 0)
        {
            return i + 1;
        }
    }
    return -1;
}

void generic(struct command_struc *command_exec, char **command, int len)
{
    command_exec->name = command[0];
    if (command[1] == NULL)
    {
        command_exec->arglist = command;
        command_exec->arglen = 1;
        command_exec->isInputRedirec = 0;
        command_exec->isOutputRedirec = 0;
    }
    else
    {
        int fd_input;
        command_exec->isInputRedirec = 0;
        command_exec->isOutputRedirec = 0;

        int inp_rid = inputredirection(command, len);
        if (inp_rid != -1)
        {
            fd_input = open(command[inp_rid], O_RDONLY);
            if (fd_input < 0)
            {
                fprintf(stderr, "error no = %s\n", strerror(errno));
                command_exec->isToBeExecuted = 0;
                return;
            }
            command_exec->isInputRedirec = 1;
            command_exec->input = fd_input;
            command[inp_rid] = NULL;
            command[inp_rid - 1] = NULL;
        }
        int fd_output;
        int out_rid = outputredirection(command, len);
        if (out_rid != -1)
        {
            fd_output = open(command[out_rid], O_WRONLY | O_CREAT, 0666);
            if (fd_output < 0)
            {
                fprintf(stderr, "error no = %s\n", strerror(errno));
                command_exec->isToBeExecuted = 0;
                return;
            }
            command[out_rid] = NULL;
            command[out_rid - 1] = NULL;
            command_exec->isOutputRedirec = 1;
            command_exec->output = fd_output;
        }
        command_exec->arglen = form_arguments(command, len);
        command_exec->arglist = command;
    }
    command_exec->isToBeExecuted = 1;
}
void checkcpupercentage(struct command_struc *command_exec, char **command, int len)
{
}
void shell_exit(struct command_struc *command_exec, char **command, int len)
{
    command_exec->isToBeExecuted = 0;
}
void checkresidentmemory(struct command_struc *command_exec, char **command, int len)
{
    // solve the memory issue
    generic(command_exec, command, len);
    if (command_exec->arglen != 2)
    {
        command_exec->isToBeExecuted = 0;
        printf("Illegal command or arguments\n");
        return;
    }
    else
    {
        command_exec->arglist = (char **)malloc(4 * sizeof(char *));
        command_exec->arglist[0] = "ps";
        command_exec->arglist[1] = command[1];
        command_exec->arglist[2] = "-orss=";
        command_exec->arglist[3] = NULL;
        command_exec->arglen = 3;
    }
}

void grep(struct command_struc *command_exec, char **command, int len)
{
    generic(command_exec, command, len);
}

void cat(struct command_struc *command_exec, char **command, int len)
{
    generic(command_exec, command, len);
}
void executeCommands(struct command_struc *command_exec, char **command, int len)
{
    int inp_rid = inputredirection(command, len);
    int fd_input = -1;
    if (inp_rid == -1)
    {
        fd_input = open(command[1], O_RDONLY);
    }
    else
    {
        fd_input = open(command[inp_rid], O_RDONLY);
        command[inp_rid - 1] = command[inp_rid];
        command[inp_rid] = NULL;
    }
    if (fd_input < 0)
    {
        fprintf(stderr, "error no = %s\n", strerror(errno));
        command_exec->isToBeExecuted = 0;
        return;
    }
    int fd_output = -1;
    int out_rid = outputredirection(command, len);
    command_exec->name = command[0];
    command_exec->arglist = command;
    command_exec->isInputRedirec = 1;
    command_exec->input = fd_input;
    command_exec->isToBeExecuted = 1;
    if (out_rid == -1)
    {
        command_exec->isOutputRedirec = 0;
        command_exec->output = 1;
    }
    else
    {
        fd_output = open(command[out_rid], O_WRONLY | O_CREAT, 0777);
        if (fd_output < 0)
        {
            fprintf(stderr, "error no = %s\n", strerror(errno));
            return;
        }
        command_exec->isOutputRedirec = 1;
        command_exec->output = fd_output;
        ;
    }
    command_exec->arglist[0] = "./shell";
    command_exec->arglist[1] = "b";
    command_exec->arglist[2] = NULL;
    command_exec->arglen = 2;
}
void listFiles(struct command_struc *command_exec, char **command, int len)
{
    int out_rid = outputredirection(command, len);
    int fd_output = -1;
    if (out_rid == -1)
    {
        fd_output = open("./files.txt", O_WRONLY | O_CREAT, 0777);
    }
    else
    {
        fd_output = open(command[out_rid], O_WRONLY | O_CREAT, 0777);
        command[out_rid - 1] = NULL;
    }

    if (fd_output < 0)
    {
        fprintf(stderr, "error no = %s\n", strerror(errno));
        return;
    }
    command_exec->name = command[0];
    command_exec->arglist = command;
    command_exec->arglen = len;
    command_exec->isInputRedirec = 0;
    command_exec->input = 0;
    command_exec->isOutputRedirec = 1;
    command_exec->output = fd_output;

    command_exec->arglist[0] = "ls";
    command_exec->arglist[1] = NULL;
    command_exec->isToBeExecuted = 1;
}
void sortFile(struct command_struc *command_exec, char **command, int len)
{
    int inp_rid = inputredirection(command, len);
    int fd_input = -1;
    if (inp_rid == -1)
    {
        fd_input = open(command[1], O_RDONLY);
    }
    else
    {
        fd_input = open(command[inp_rid], O_RDONLY);
        command[inp_rid - 1] = command[inp_rid];
        command[inp_rid] = NULL;
    }
    if (fd_input < 0)
    {
        fprintf(stderr, "error no = %s\n", strerror(errno));
        command_exec->isToBeExecuted = 0;
        return;
    }

    int fd_output = -1;
    int out_rid = outputredirection(command, len);
    command_exec->name = command[0];
    command_exec->arglist = command;
    command_exec->arglen = len;

    command_exec->isInputRedirec = 1;
    command_exec->input = fd_input;
    command_exec->isToBeExecuted = 1;
    if (out_rid == -1)
    {
        command_exec->isOutputRedirec = 0;
        command_exec->output = 1;
    }
    else
    {
        fd_output = open(command[out_rid], O_WRONLY | O_CREAT, 0777);
        if (fd_output < 0)
        {
            fprintf(stderr, "error no = %s\n", strerror(errno));
            return;
        }
        command_exec->isOutputRedirec = 1;
        command_exec->output = fd_output;
        ;
    }

    command_exec->arglist[0] = "sort";
    command_exec->arglist[2] = NULL;
}

int checkIfValidCommand(char *commandName)
{
    int pos = 0;
    int isValid = -1;
    while (supportedCommands[pos] != NULL)
    {
        if (strcmp(commandName, supportedCommands[pos]) == 0)
        {
            isValid = pos;
            break;
        }
        pos++;
    }
    return isValid;
}

command_func_ptr command_functions[8] = {
    checkcpupercentage,
    checkresidentmemory,
    listFiles,
    sortFile,
    executeCommands,
    grep,
    cat,
    shell_exit};
void execute_command(struct command_struc *command, struct pipe_struc *pipeConfig)
{
    pid_t pid;
    int status;
    int pfd[2];
    if (pipeConfig != NULL && pipeConfig->ispipe == 1)
    {
        pipe(pfd);
    }
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "error no = %s\n", strerror(errno));
    }
    else if (pid == 0)
    {
        if (pipeConfig != NULL && pipeConfig->ispipe == 1)
        {
            dup2(pipeConfig->readend, 0);
            if (pipeConfig->isLast == 0)
                dup2(pfd[1], 1);
            close(pfd[0]);
            close(pfd[1]);
        }
        if (command->isInputRedirec == 1)
        {
            dup2(command->input, 0);
        }
        if (command->isOutputRedirec == 1)
        {
            dup2(command->output, 1);
        }
        if (execvp(command->arglist[0], command->arglist) < 0)
        {
            perror("Illegal command or arguments");
        }
    }
    else
    {
        waitpid(pid, &status, 0);
        if (pipeConfig != NULL && pipeConfig->ispipe == 1)
        {
            close(pfd[1]);
            pipeConfig->readend = pfd[0];
        }
    }
}
char *read_line(FILE *file)
{
    char *ln = NULL;
    size_t n = 0;
    if (getline(&ln, &n, file) == -1)
    {
        return NULL;
    }
    return ln;
}
int tokenize(char *ln, char *formated_ln, char **command1, int *len1, char **command2, int *len2)
{

    int pos = 0;
    int isSemiFound = 0;
    int len = strlen(ln);
    formated_ln = (char *)malloc(2 * len * sizeof(char));
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (ln[i] == '|' || ln[i] == ';' || ln[i] == '<' || ln[i] == '>')
        {
            formated_ln[j++] = ' ';
            formated_ln[j++] = ln[i];
            if (i != len - 1 && ln[i + 1] == '>')
                formated_ln[j++] = ln[i + 1];
            formated_ln[j++] = ' ';
        }
        else
        {
            formated_ln[j++] = ln[i];
        }
    }
    char *tok;
    tok = strtok(formated_ln, DELE);
    while (tok != NULL)
    {
        if (*tok == ';')
        {
            isSemiFound = 1;
            command1[pos] = NULL;
            *len1 = pos;
            pos = 0;
        }
        else if (!isSemiFound)
        {
            command1[pos++] = tok;
        }
        else
        {

            command2[pos++] = tok;
        }
        tok = strtok(NULL, DELE);
    }
    if (isSemiFound)
    {
        command2[pos] = NULL;
        *len2 = pos;
    }
    else
    {
        command1[pos] = NULL;
        *len1 = pos;
        *len2 = 0;
    }
    return isSemiFound;
}
void printSupportedCommand()
{
    int i = 0;
    printf("Following are the supported commands :\n");
    while (supportedCommands[i] != NULL)
    {
        printf("%d. %s\n", i, supportedCommands[i]);
        i++;
    }
}
void closeAllFiles(struct command_struc *command)
{
    if (command->isInputRedirec == 1)
    {
        close(command->input);
    }
    if (command->isOutputRedirec)
    {
        close(command->output);
    }
}
int checkPipe(char **arg)
{
    int i = 0, count = 0;
    for (i = 0; arg[i] != NULL; i++)
    {
        if (strcmp(arg[i], "|") == 0)
        {
            count++;
        }
    }
    return count;
}

void execute_single_command(char **command1, int len1)
{
    int pipecnt = 0;
    int cnt = 0;
    if ((pipecnt = checkPipe(command1)))
    {
        int *startpos = (int *)malloc((pipecnt + 1) * sizeof(int));
        int *commandindices = (int *)malloc((pipecnt + 1) * sizeof(int));
        startpos[0] = 0;
        for (int i = 0; command1[i] != NULL; i++)
        {
            if (strcmp(command1[i], "|") == 0)
            {
                command1[i] = NULL;
                commandindices[cnt] = checkIfValidCommand(command1[startpos[cnt]]);
                if (commandindices[cnt] == -1)
                {
                    printf("Illegal command or arguments\n");
                    fflush(stdout);
                    return;
                }
                cnt++;
                startpos[cnt] = i + 1;
            }
        }
        commandindices[cnt] = checkIfValidCommand(command1[startpos[cnt]]);
        if (commandindices[cnt] == -1)
        {
            printf("Illegal command or arguments\n");
            fflush(stdout);
            return;
        }

        struct pipe_struc pipeConfig;
        pipeConfig.ispipe = 1;
        pipeConfig.readend = 0;
        pipeConfig.isLast = 0;
        for (int i = 0; i <= cnt; i++)
        {
            struct command_struc command_exec;

            int length = 0;
            if (i != cnt)
            {
                length = startpos[i + 1] - startpos[i] - 1;
            }
            else
            {
                pipeConfig.isLast = 1;
                length = len1 - startpos[cnt];
            }
            command_functions[commandindices[i]](&command_exec, command1 + startpos[i], length);

            if (command_exec.isToBeExecuted == 1)
                execute_command(&command_exec, &pipeConfig);
            closeAllFiles(&command_exec);
        }
    }
    else
    {
        int commandIndex = checkIfValidCommand(command1[0]);
        if (commandIndex == -1)
        {
            printf("Illegal command or arguments\n");
            fflush(stdout);
        }
        else
        {
            struct command_struc command_exec;
            command_functions[commandIndex](&command_exec, command1, len1);
            if (command_exec.isToBeExecuted == 1)
                execute_command(&command_exec, NULL);
            closeAllFiles(&command_exec);
        }
    }
}
int execute_commands(char **command1, int len1, char **command2, int len2, int isSemFound)
{
    if (strcmp(command1[0], "exit") == 0 && len1 == 1)
    {
        return -1;
    }
    if (!isSemFound)
    {

        execute_single_command(command1, len1);
    }
    else
    {

        int pid;
        int status;
        pid = fork();
        if (pid < 0)
        {
            perror("Fork Failed");
        }
        else if (pid == 0)
        {
            if (strcmp(command2[0], "exit") == 0 && len2 == 1)
            {
                exit(-1);
            }
            execute_single_command(command2, len2);
            exit(-1);
        }
        else
        {
            execute_single_command(command1, len1);
            waitpid(pid, &status, 0);
            if (strcmp(command2[0], "exit") == 0 && len2 == 1)
            {
                return -1;
            }
        }
    }
    return 1;
}
int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "b") == 0)
    {
        isBatchMode = 1;
    }
    if (!isBatchMode)
        printSupportedCommand();
    char *ln;
    char *formatted_ln;
    char *command1[MAX_TOKENS];
    int len1, len2;
    char *command2[MAX_TOKENS];
    do
    {
        // read the input
        if (!isBatchMode)
            printf("myShell>");

        if ((ln = read_line(stdin)) == NULL)
        {
            isBatchMode = 0;
            break;
        }

        int isSemiFound = tokenize(ln, formatted_ln, command1, &len1, command2, &len2);
        if (len1 == 0)
        {
            continue;
        }
        int st = execute_commands(command1, len1, command2, len2, isSemiFound);
        free(ln);
        free(formatted_ln);
        fflush(stdout);
        if (st == -1)
        {
            exit(0);
        }
    } while (1);
}
