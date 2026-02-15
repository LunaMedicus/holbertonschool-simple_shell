#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

/**
 * tokenize_line - parse one input line into argv form for execution
 * @line: mutable command line buffer
 * @prog: shell program name for diagnostics
 * @argv_exec: output argv vector storage
 * @status: status output used for allocation failures
 * Return: 1 if a command was parsed, 0 otherwise
 */
int tokenize_line(char *line, char *prog, char ***argv_exec, int *status);

/**
 * handle_builtin - execute supported builtin commands when matched
 * @argv_exec: parsed command vector
 * @status: shell status storage
 * @should_exit: set to 1 when shell must terminate
 * Return: 1 if builtin was handled, 0 if command is external
 */
int handle_builtin(char **argv_exec, int *status, int *should_exit);

/**
 * print_env - print the process environment to standard output
 */
void print_env(void);

#endif
