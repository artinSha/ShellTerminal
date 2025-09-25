#ifndef HISTORY_H
#define HISTORY_H

#define MAX_HISTORY 50
#define MAX_ARGS 256

void add_to_history(char **args, int arg_count);
void display_history(void);
char **find_command_by_number(int cmd_num, int *arg_count);
char **get_last_command(int *arg_count);
int is_bang_command(char *cmd);

#endif
