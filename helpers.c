#include "shell.h"

/**
 * print_env - print current process environment variables
 *
 * Iterates over global environ and writes one "KEY=VALUE" entry per line.
 * Used by the `env` builtin.
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
 * grow_argv - resize token array when current capacity is exhausted
 * @argv_exec: address of argv pointer to reallocate
 * @argv_cap: pointer to current argv capacity (doubled on success)
 * @prog: program name
 * @status: shell status updated to 1 on allocation failure
 *
 * On realloc failure, this function frees the existing argv buffer because the
 * caller cannot continue token collection without storage.
 * Return: 1 on successful resize, 0 on failure
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
 * tokenize_line - split raw input into an exec-ready argv array
 * @line: mutable input line buffer (modified in place by strtok)
 * @prog: program name
 * @argv_exec: output location for allocated NULL-terminated argv array
 * @status: shell status updated on memory allocation errors
 *
 * Ignores empty lines and full-line comments beginning with '#'. Tokens are
 * separated on spaces and tabs only, with dynamic growth as needed.
 * Return: 1 when at least one tokenized command is available, else 0
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
 * handle_builtin - dispatch builtins recognized by this shell
 * @argv_exec: parsed command vector, command name at index 0
 * @status: reserved for builtin status propagation
 * @should_exit: output flag set to 1 when `exit` is requested
 *
 * Supported builtins:
 * - exit: request termination of the shell loop
 * - env: print environment variables
 * Return: 1 when command was handled as a builtin, otherwise 0
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
