#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int tokenize_line(char *line, char *prog, char ***argv_exec, int *status);
int handle_builtin(char **argv_exec, int *status, int *should_exit);
void print_env(void);

#endif
