#include "history.h"
#include "msgs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char *arguments[MAX_ARGS];
  int arg_count;
  int number;
} HistoryEntry;

static HistoryEntry history[MAX_HISTORY];
static int history_count = 0;
static int history_size = 0;

// helper function to go from num to string
// I used this format instead of the given header in msgs.h
int int_to_string(int num, char *buffer) {
  if (num == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return 1;
  }

  int len = 0;
  int temp = num;
  int is_negative = 0;

  if (num < 0) {
    is_negative = 1;
    num = -num;
  }

  while (temp != 0) {
    len++;
    temp /= 10;
  }
  if (is_negative)
    len++;

  int index = len - 1;
  buffer[len] = '\0';

  while (num != 0) {
    buffer[index--] = (num % 10) + '0';
    num /= 10;
  }

  if (is_negative) {
    buffer[0] = '-';
  }

  return len;
}

void add_to_history(char **args, int arg_count) {
  // if empty command
  if (arg_count == 0 || args[0] == NULL)
    return;

  // Don't add bang commands
  if (is_bang_command(args[0]))
    return;

  history[history_size].arg_count = arg_count;
  history[history_size].number = history_count;

  // Copy the arguments over
  for (int i = 0; i < arg_count && i < MAX_ARGS; i++) {

    // Check if array is NULL at current position
    if (args[i] == NULL) {
      history[history_size].arguments[i] = NULL;
      continue;
    }

    history[history_size].arguments[i] = malloc(strlen(args[i]) + 1);
    strcpy(history[history_size].arguments[i], args[i]);
  }

  // Set rest of the pointers to NULL
  for (int i = arg_count; i < MAX_ARGS; i++) {
    history[history_size].arguments[i] = NULL;
  }

  history_size++;
  history_count++;
}

void display_history(void) {
  // This line I found online, it clamps the lower bound to 0 :)
  int start = (history_size >= 10) ? history_size - 10 : 0;

  // loop through 10 most recent history
  for (int i = history_size - 1; i >= start; i--) {
    char num_buffer[20];
    int num_len = int_to_string((history[i].number), num_buffer);
    write(STDOUT_FILENO, num_buffer, num_len);
    write(STDOUT_FILENO, "\t", 1);

    // Print all args
    for (int j = 0; j < history[i].arg_count; j++) {
      write(STDOUT_FILENO, history[i].arguments[j],
            strlen(history[i].arguments[j]));
      if (j < history[i].arg_count - 1) {
        write(STDOUT_FILENO, " ", 1);
      }
    }
    write(STDOUT_FILENO, "\n", 1);
  }
}

char **find_command_by_number(int cmd_number, int *arg_count) {
  // Travel through the array to find the proper history entry
  for (int i = 0; i < history_size; i++) {
    if (history[i].number == cmd_number) {
      *arg_count = history[i].arg_count;
      return history[i].arguments;
    }
  }
  // if nothing found, return NULL
  *arg_count = 0;
  return NULL;
}

char **get_last_command(int *arg_count) {
  if (history_size == 0) {
    *arg_count = 0;
    return NULL;
  }
  *arg_count = history[history_size - 1].arg_count;
  return history[history_size - 1].arguments;
}

// helper for checking if something is a '!' command
int is_bang_command(char *cmd) { return (cmd[0] == '!'); }
