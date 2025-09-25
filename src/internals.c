#include "internals.h"
#include "history.h"
#include "msgs.h"
#include "shell.h"
#include <ctype.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Save a pointer to the previous directory here for 'cd -'
static char *prev_dir = NULL;

// Wrapper for chdir() to avoid repeated code
void changeDir(const char *path) {
  // Save current directory before changing
  char *current = getcwd(NULL, 0);

  if (chdir(path) == -1) {
    // chdir() failed
    const char *msg = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
  } else {
    if (prev_dir) {
      free(prev_dir);
    }
    prev_dir = current;
  }
}

// helper for writing out arguments
void write_arguments(char **args) {
  for (int i = 0; args[i] != NULL; i++) {
    write(STDOUT_FILENO, args[i], strlen(args[i]));

    // add space
    if (args[i + 1] != NULL) {
      write(STDOUT_FILENO, " ", 1);
    }
  }

  write(STDOUT_FILENO, "\n", 1);
}
// helper for showing all help messages
void displayAllHelp() {
  char *msg = FORMAT_MSG("exit", EXIT_HELP_MSG);
  write(STDOUT_FILENO, msg, strlen(msg));
  msg = FORMAT_MSG("help", HELP_HELP_MSG);
  write(STDOUT_FILENO, msg, strlen(msg));
  msg = FORMAT_MSG("pwd", PWD_HELP_MSG);
  write(STDOUT_FILENO, msg, strlen(msg));
  msg = FORMAT_MSG("cd", CD_HELP_MSG);
  write(STDOUT_FILENO, msg, strlen(msg));
  msg = FORMAT_MSG("history", HISTORY_HELP_MSG);
  write(STDOUT_FILENO, msg, strlen(msg));
}

void addToHistory(char **arguments, int arg_count, bool background) {
  if (background) {
    arguments[arg_count] = "&";
    arg_count++;
  }
  add_to_history(arguments, arg_count);
}

bool checkInternalCommands(char **arguments, int arg_count, bool background) {
  // exit
  if (strcmp(arguments[0], "exit") == 0) {
    if (arg_count > 1) {
      const char *msg = FORMAT_MSG("exit", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      return true;
    }
    // Kill program
    exit(0);
  }
  // pwd
  else if (strcmp(arguments[0], "pwd") == 0) {
    if (arg_count > 1) {
      const char *msg = FORMAT_MSG("pwd", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else {
      addToHistory(arguments, arg_count, background);
      char dir_buffer[256];

      if (getcwd(dir_buffer, sizeof(dir_buffer)) != NULL) {
        write(STDOUT_FILENO, dir_buffer, strlen(dir_buffer));
        write(STDOUT_FILENO, "\n", 1);
      } else {
        const char *msg = FORMAT_MSG("pwd", GETCWD_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
      }
    }
    return true;
  }
  //!
  else if (arguments[0][0] == '!') {
    // check '!!'
    if (strcmp(arguments[0], "!!") == 0) {
      // get last command
      int argCount;
      char **args = get_last_command(&argCount);
      // If returns NULL, then history size == 0
      if (args == NULL) {
        const char *msg = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
        return true;
      }
      write_arguments(args);
      runProcess(args, argCount);
      return true;

    } else if (arguments[0][1] != '\0') {
      char *num_str = arguments[0] + 1;

      // Here we check if n is a digit
      int is_valid = 1;
      for (int i = 0; num_str[i] != '\0'; i++) {
        if (!isdigit(num_str[i])) {
          is_valid = 0;
          break;
        }
      }

      if (!is_valid) {
        const char *msg = FORMAT_MSG("history", HISTORY_INVALID_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
        return true;
      }

      // conver the string 'n' toa number
      int cmd_num = atoi(num_str);

      int argCount = 0;
      char **args = find_command_by_number(cmd_num, &argCount);
      // If null, command wasn't found or was invalid
      if (args == NULL) {
        const char *msg = FORMAT_MSG("history", HISTORY_INVALID_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
        return true;
      }
      write_arguments(args);
      runProcess(args, argCount);
      return true;
    }
  }
  // cd
  else if (strcmp(arguments[0], "cd") == 0) {
    if (arg_count > 2) {
      const char *msg = FORMAT_MSG("cd", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else if (arg_count == 1) {
      // No arguments passed -> go home
      struct passwd *pw = getpwuid(getuid());
      changeDir(pw->pw_dir);
    } else if (*arguments[1] == '~') {
      // Handle tilda
      struct passwd *pw = getpwuid(getuid());
      if (strlen(arguments[1]) == 1) {
        changeDir(pw->pw_dir);
      } else {
        // construct path
        char full_path[256];
        strcpy(full_path, pw->pw_dir);
        strcat(full_path, arguments[1] + 1);
        changeDir(full_path);
      }
    } else if (*arguments[1] == '-') {
      if (prev_dir == NULL) {
        // handle the case where no previous directory exists
        const char *msg = FORMAT_MSG("cd", "No previous directory exists!");
        write(STDERR_FILENO, msg, strlen(msg));
      } else {
        changeDir(prev_dir);
      }
    } else {
      // Change to directory given as argument
      changeDir(arguments[1]);
    }
    addToHistory(arguments, arg_count, background);
    return true;
  }
  // history
  else if (strcmp(arguments[0], "history") == 0) {
    addToHistory(arguments, arg_count, background);
    display_history();
    return true;
  }
  // help
  else if (strcmp(arguments[0], "help") == 0) {
    if (arg_count > 2) {
      const char *msg = FORMAT_MSG("help", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else if (arg_count == 1) {
      displayAllHelp();
    } else if (strcmp(arguments[1], "exit") == 0) {
      const char *msg = FORMAT_MSG("exit", EXIT_HELP_MSG);
      write(STDOUT_FILENO, msg, strlen(msg));
    } else if (strcmp(arguments[1], "help") == 0) {
      const char *msg = FORMAT_MSG("help", HELP_HELP_MSG);
      write(STDOUT_FILENO, msg, strlen(msg));
    } else if (strcmp(arguments[1], "pwd") == 0) {
      const char *msg = FORMAT_MSG("pwd", PWD_HELP_MSG);
      write(STDOUT_FILENO, msg, strlen(msg));
    } else if (strcmp(arguments[1], "cd") == 0) {
      const char *msg = FORMAT_MSG("cd", CD_HELP_MSG);
      write(STDOUT_FILENO, msg, strlen(msg));
    } else if (strcmp(arguments[1], "history") == 0) {
      const char *msg = FORMAT_MSG("history", HISTORY_HELP_MSG);
      write(STDOUT_FILENO, msg, strlen(msg));
    } else {
      // external command
      /* Need to define our own here, because FORMAT_MSG doesn't work with
       variables */
      write(STDOUT_FILENO, arguments[1], strlen(arguments[1]));
      write(STDOUT_FILENO, ": ", 2);
      write(STDOUT_FILENO, EXTERN_HELP_MSG, strlen(EXTERN_HELP_MSG));
      write(STDOUT_FILENO, "\n", 1);
    }
    addToHistory(arguments, arg_count, background);
    return true;
  }
  // If none of the above flags hit, return false
  return false;
}
