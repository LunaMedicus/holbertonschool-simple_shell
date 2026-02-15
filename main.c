#include "shell.h"

/**
 * print_not_found - prints command not found error
 * @prog: program name
 * @line: input line number
 * @cmd: command name
 */
static void print_not_found(char *prog, unsigned long int line, char *cmd)
{
fprintf(stderr, "%s: %lu: %s: not found\n", prog, line, cmd);
}

/**
 * resolve_command - resolves executable path from command
 * @cmd: command name
 * Return: allocated executable path or NULL
 */
static char *resolve_command(char *cmd)
{
char *path, *path_copy, *dir, *full;
size_t len;

if (strchr(cmd, '/') != NULL)
{
if (access(cmd, X_OK) == 0)
return (strdup(cmd));
return (NULL);
}
path = getenv("PATH");
if (path == NULL)
return (NULL);
path_copy = strdup(path);
if (path_copy == NULL)
return (NULL);
dir = strtok(path_copy, ":");
while (dir != NULL)
{
len = strlen(dir) + strlen(cmd) + 2;
full = malloc(len);
if (full == NULL)
{
free(path_copy);
return (NULL);
}
sprintf(full, "%s/%s", dir, cmd);
if (access(full, X_OK) == 0)
{
free(path_copy);
return (full);
}
free(full);
dir = strtok(NULL, ":");
}
free(path_copy);
return (NULL);
}

/**
 * run_command - executes a command in child process
 * @prog: program name
 * @line: input line number
 * @cmd: command name
 * Return: child exit status
 */
static int run_command(char *prog, unsigned long int line, char *cmd)
{
pid_t pid;
int status;
char *path;
char *argv_exec[2];

path = resolve_command(cmd);
if (path == NULL)
{
print_not_found(prog, line, cmd);
return (127);
}
pid = fork();
if (pid == -1)
{
perror(prog);
free(path);
return (1);
}
if (pid == 0)
{
argv_exec[0] = cmd;
argv_exec[1] = NULL;
execve(path, argv_exec, environ);
perror(prog);
free(path);
exit(126);
}
free(path);
waitpid(pid, &status, 0);
if (WIFEXITED(status))
return (WEXITSTATUS(status));
return (1);
}

/**
 * shell_loop - reads and executes command lines
 * @prog: program name
 */
static void shell_loop(char *prog)
{
char *line, *cmd, *status_token, **env;
size_t len;
ssize_t read_len;
unsigned long int line_no;
int status;

line = NULL;
len = 0;
line_no = 0;
status = 0;
while (1)
{
if (isatty(STDIN_FILENO))
write(STDOUT_FILENO, "($) ", 4);
read_len = getline(&line, &len, stdin);
if (read_len == -1)
{
if (isatty(STDIN_FILENO))
write(STDOUT_FILENO, "\n", 1);
break;
}
line_no++;
if (read_len > 0 && line[read_len - 1] == '\n')
line[read_len - 1] = '\0';
cmd = strtok(line, " \t");
if (cmd == NULL)
continue;
if (strcmp(cmd, "exit") == 0)
{
status_token = strtok(NULL, " \t");
if (status_token != NULL)
status = atoi(status_token);
break;
}
if (strcmp(cmd, "env") == 0)
{
env = environ;
while (*env != NULL)
{
printf("%s\n", *env);
env++;
}
continue;
}
status = run_command(prog, line_no, cmd);
}
free(line);
exit(status);
}

/**
 * main - entry point of the shell
 * @argc: number of args
 * @argv: argument vector
 * Return: always 0
 */
int main(int argc, char **argv)
{
(void)argc;
shell_loop(argv[0]);
return (0);
}
