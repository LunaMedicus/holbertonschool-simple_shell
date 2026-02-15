#include "shell.h"

/**
 * print_env - prints the environment
 */
void print_env(void)
{
char **env;

env = environ;
while (*env != NULL)
{
printf("%s\n", *env);
env++;
}
}

/**
 * grow_argv - grows argv buffer
 * @argv_exec: argv buffer address
 * @argv_cap: argv capacity
 * @prog: program name
 * @status: shell status
 * Return: 1 on success, 0 on failure
 */
static int grow_argv(char ***argv_exec, size_t *argv_cap,
char *prog, int *status)
{
char **tmp;

*argv_cap *= 2;
tmp = realloc(*argv_exec, *argv_cap * sizeof(char *));
if (tmp == NULL)
{
free(*argv_exec);
*argv_exec = NULL;
perror(prog);
*status = 1;
return (0);
}
*argv_exec = tmp;
return (1);
}

/**
 * tokenize_line - tokenizes input line into argv array
 * @line: input line
 * @prog: program name
 * @argv_exec: output argv array
 * @status: shell status to update on allocation failure
 * Return: 1 if command exists, 0 otherwise
 */
int tokenize_line(char *line, char *prog, char ***argv_exec, int *status)
{
char *token;
size_t argv_cap, argc_exec;

if (line[0] == '\0' || line[0] == '#')
return (0);
argv_cap = 8;
argc_exec = 0;
*argv_exec = malloc(argv_cap *sizeof(char *));
if (*argv_exec == NULL)
{
perror(prog);
*status = 1;
return (0);
}
token = strtok(line, " \t");
while (token != NULL)
{
if (argc_exec + 1 >= argv_cap)
{
if (grow_argv(argv_exec, &argv_cap, prog, status) == 0)
return (0);
}
(*argv_exec)[argc_exec++] = token;
token = strtok(NULL, " \t");
}
if (argc_exec == 0)
{
free(*argv_exec);
*argv_exec = NULL;
return (0);
}
(*argv_exec)[argc_exec] = NULL;
return (1);
}

/**
 * handle_builtin - handles shell builtins
 * @argv_exec: command arguments
 * @status: shell status
 * @should_exit: set to 1 when shell must exit
 * Return: 1 if builtin handled, 0 otherwise
 */
int handle_builtin(char **argv_exec, int *status, int *should_exit)
{
(void)status;

if (strcmp(argv_exec[0], "exit") == 0)
{
*should_exit = 1;
return (1);
}
if (strcmp(argv_exec[0], "env") == 0)
{
print_env();
return (1);
}
return (0);
}
