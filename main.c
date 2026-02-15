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
		snprintf(full, len, "%s/%s", dir, cmd);
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
 * @argv_exec: command arguments vector
 * Return: child exit status
 */
static int run_command(char *prog, unsigned long int line, char **argv_exec)
{
pid_t pid;
int status;
char *path;

path = resolve_command(argv_exec[0]);
if (path == NULL)
{
print_not_found(prog, line, argv_exec[0]);
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
char *line, **argv_exec;
size_t len;
ssize_t read_len;
unsigned long int line_no;
int status, should_exit;

line = NULL;
len = 0;
line_no = 0;
status = 0;
argv_exec = NULL;
while (1)
{
if (isatty(STDIN_FILENO))
{
write(STDOUT_FILENO, "($) ", 4);
}
read_len = getline(&line, &len, stdin);
if (read_len == -1)
{
if (isatty(STDIN_FILENO))
{
write(STDOUT_FILENO, "\n", 1);
}
break;
}
line_no++;
if (read_len > 0 && line[read_len - 1] == '\n')
line[read_len - 1] = '\0';
if (tokenize_line(line, prog, &argv_exec, &status) == 0)
continue;
should_exit = 0;
if (handle_builtin(argv_exec, &status, &should_exit) == 0)
status = run_command(prog, line_no, argv_exec);
free(argv_exec);
argv_exec = NULL;
if (should_exit)
break;
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
