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
 * is_executable_file - checks if path is an executable regular file
 * @path: file path
 * Return: 1 if executable regular file, 0 otherwise
 */
static int is_executable_file(char *path)
{
struct stat st;

if (access(path, X_OK) != 0)
return (0);
if (stat(path, &st) == -1)
return (0);
return (S_ISREG(st.st_mode));
}

/**
 * build_path_candidate - builds candidate path for command lookup
 * @dir_start: start of path directory segment
 * @dir_len: length of path directory segment
 * @cmd: command name
 * Return: allocated candidate path or NULL
 */
static char *build_path_candidate(char *dir_start, size_t dir_len, char *cmd)
{
char *candidate;
size_t cmd_len, total_len;

cmd_len = strlen(cmd);
if (dir_len == 0)
{
total_len = cmd_len + 3;
candidate = malloc(total_len);
if (candidate == NULL)
return (NULL);
candidate[0] = '.';
candidate[1] = '/';
memcpy(candidate + 2, cmd, cmd_len + 1);
return (candidate);
}
total_len = dir_len + cmd_len + 2;
candidate = malloc(total_len);
if (candidate == NULL)
return (NULL);
memcpy(candidate, dir_start, dir_len);
candidate[dir_len] = '/';
memcpy(candidate + dir_len + 1, cmd, cmd_len + 1);
return (candidate);
}

/**
 * resolve_command - resolves executable path from command
 * @cmd: command name
 * Return: allocated executable path or NULL
 */
static char *resolve_command(char *cmd)
{
char *path, *cursor, *dir_start, *candidate;
size_t dir_len;

if (strchr(cmd, '/') != NULL)
{
if (is_executable_file(cmd))
return (strdup(cmd));
return (NULL);
}
path = getenv("PATH");
if (path == NULL)
return (NULL);
dir_start = path;
cursor = path;
while (1)
{
if (*cursor == ':' || *cursor == '\0')
{
dir_len = cursor - dir_start;
candidate = build_path_candidate(dir_start, dir_len, cmd);
if (candidate == NULL)
return (NULL);
if (is_executable_file(candidate))
return (candidate);
free(candidate);
if (*cursor == '\0')
break;
dir_start = cursor + 1;
}
cursor++;
}
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
