#include "shell.h"

int run_command(char *prog, unsigned long int line, char **argv_exec);

/**
 * read_command_line - reads one command line from stdin
 * @line: line buffer
 * @len: line buffer length
 * Return: 0 on success, -1 on EOF/error
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
 * shell_loop - reads and executes command lines
 * @prog: program name
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
