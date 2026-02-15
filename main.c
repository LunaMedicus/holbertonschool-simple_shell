#include "shell.h"

/**
 * print_not_found - emit the standard shell-style "command not found" error
 * @prog: program name
 * @line: command counter used in error reporting
 * @cmd: command that failed path resolution
 *
 * Format intentionally matches project requirement:
 * "<prog>: <line>: <cmd>: not found"
 */
static void print_not_found(char *prog, unsigned long int line, char *cmd)
{
fprintf(stderr, "%s: %lu: %s: not found\n", prog, line, cmd);
}

/**
 * is_executable_file - validate that a path points to a runnable file
 * @path: candidate filesystem path
 *
 * First checks execute permission with access(2), then confirms the path is a
 * regular file via stat(2). This rejects directories and special files.
 * Return: 1 when @path is executable and regular, otherwise 0
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
 * build_path_candidate - assemble one executable candidate from PATH + command
 * @dir_start: start of current PATH segment
 * @dir_len: length of current PATH segment
 * @cmd: command name to append
 *
 * Empty PATH segments are treated as the current directory and expanded as
 * "./<cmd>". Returned string is heap-allocated and must be freed by caller.
 * Return: allocated candidate string on success, or NULL on allocation failure
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
 * get_env_value - fetch environment variable value directly from environ
 * @name: variable name
 *
 * Performs a linear scan of environ for a "name=value" entry and returns a
 * pointer to the value region inside environ storage (not a copy).
 * Return: pointer to value if found, otherwise NULL
 */
static char *get_env_value(char *name)
{
char **envp;
size_t name_len;

name_len = strlen(name);
envp = environ;
while (*envp != NULL)
{
if (strncmp(*envp, name, name_len) == 0 && (*envp)[name_len] == '=')
return (*envp + name_len + 1);
envp++;
}
return (NULL);
}

/**
 * resolve_command - resolve a command name into an executable absolute/relative path
 * @cmd: command name
 *
 * If @cmd contains '/', it is treated as an explicit path and validated
 * directly. Otherwise, each directory from PATH is searched left-to-right until
 * an executable regular file is found. Returned string is allocated and owned
 * by the caller.
 * Return: allocated resolved path on success, or NULL if not found/error
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
path = get_env_value("PATH");
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
 * run_command - execute a non-builtin command using fork/execve
 * @prog: program name
 * @line: command counter for error reporting
 * @argv_exec: NULL-terminated argument vector (argv_exec[0] is command)
 *
 * Resolves the command path, forks a child, executes via execve(2), and waits
 * in the parent for completion. Mirrors shell semantics by returning 127 for
 * command-not-found and child's exit code when available.
 * Return: resulting status code to keep as shell status
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
 * read_command_line - read and normalize one input line from standard input
 * @line: getline-managed buffer pointer (allocated/reused by getline)
 * @len: size of buffer tracked for getline
 *
 * Displays prompt in interactive mode, reads one line with getline(3), and
 * strips the trailing newline when present.
 * Return: 0 when a line was read, -1 on EOF/read failure
 */
static int read_command_line(char **line, size_t *len)
{
ssize_t read_len;

if (isatty(STDIN_FILENO))
write(STDOUT_FILENO, "($) ", 4);
read_len = getline(line, len, stdin);
if (read_len == -1)
{
if (isatty(STDIN_FILENO))
write(STDOUT_FILENO, "\n", 1);
return (-1);
}
if (read_len > 0 && (*line)[read_len - 1] == '\n')
(*line)[read_len - 1] = '\0';
return (0);
}

/**
 * shell_loop - main REPL loop for parsing, dispatching, and status tracking
 * @prog: program name
 *
 * Repeatedly reads input, tokenizes it, handles builtins, and executes external
 * commands. The function keeps the last command status and exits the process
 * with that status when input ends or an exit builtin is requested.
 */
static void shell_loop(char *prog)
{
char *line, **argv_exec;
size_t len;
unsigned long int line_no;
int status, should_exit;

line = NULL;
len = 0;
line_no = 0;
status = 0;
argv_exec = NULL;
while (1)
{
if (read_command_line(&line, &len) == -1)
break;
line_no++;
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
 * main - shell program entry point
 * @argc: number of command-line arguments (unused)
 * @argv: argument vector, argv[0] used as program name in diagnostics
 *
 * Delegates execution to shell_loop(), which owns process termination and exit
 * status. Return path is kept for completeness.
 * Return: 0 (not reached during normal shell_loop lifecycle)
 */
int main(int argc, char **argv)
{
(void)argc;
shell_loop(argv[0]);
return (0);
}
