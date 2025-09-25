#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "internals.h"
#include "msgs.h"

#define MAX_CMD_LEN 256
#define MAX_HISTORY 50

// signal handler
void sigint_handler(int sig) {
  write(STDOUT_FILENO, "\n", 1);
  displayAllHelp();
}

ssize_t getUserInput(char *readBuffer) {
  char dir_buffer[MAX_CMD_LEN];

  // Get current directory
  if (getcwd(dir_buffer, sizeof(dir_buffer)) != NULL) {
    // write command line prompt
    write(STDOUT_FILENO, dir_buffer, strlen(dir_buffer));
    write(STDOUT_FILENO, "$", 1);
    write(STDOUT_FILENO, " ", 1);
  } else {
    const char *msg = FORMAT_MSG("shell", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return 0;
  }
  return read(STDIN_FILENO, readBuffer, MAX_CMD_LEN - 1);
}

// function for running processes, background/foreground
void runProcess(char **arguments, int arg_count) {
  bool background = false;
  // Check if last token is '&'
  if (arg_count > 0 && strcmp(arguments[arg_count - 1], "&") == 0) {
    background = true;
    arguments[arg_count - 1] = NULL; // Remove '&'
    arg_count--;
  }

  // Check if the command provided is a pre-defined 'internal'
  if (checkInternalCommands(arguments, arg_count, background)) {
    return;
  }

  // Now fork a new process
  pid_t pid = fork();
  if (pid < 0) {
    // Check if fork failed
    char *msg = FORMAT_MSG("fork", FORK_ERROR_MSG);
    write(STDERR_FILENO, msg, strlen(msg));
    return;
  } else if (pid == 0) {
    // Now we use execvp because we can pass it an array of arguments
    if (execvp(arguments[0], arguments) == -1) {
      // Note that execvp only returns in case of an error
      char *msg = FORMAT_MSG("shell", EXEC_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      // Here we want to kill the child if execvp fails
      _exit(1);
    }
  } else {
    // We are in the parent
    if (background) {
      // We are running in background
      // Here we have to add to history immediately
      arguments[arg_count] = "&";
      add_to_history(arguments, arg_count + 1);
    } else {
      // We are running in foreground, wait for process to return
      int status;
      int result = waitpid(pid, &status, 0);
      // Check if waitpid failed
      if (result == -1) {
        char *msg = FORMAT_MSG("shell", WAIT_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
        return;
      }

      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        add_to_history(arguments, arg_count);
      }
    }
  }
}
int main() {
  struct sigaction act;
  act.sa_handler = sigint_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  // Make sure sigaction succeeds
  if (sigaction(SIGINT, &act, NULL) == -1) {
    write(STDERR_FILENO, "Sigint Failed!", strlen("Sigint Failed!"));
    exit(1);
  }
  while (true) {
    char buffer[MAX_CMD_LEN] = "\0";

    // read input from user
    ssize_t bytes_read = getUserInput(buffer);

    // if EINTR, meaning call was interrupted, keep reading input
    while (bytes_read < 0 && errno == EINTR) {
      bytes_read = getUserInput(buffer);
    }

    // read returns -1 upon error, perform error-check
    if (bytes_read < 0) {
      const char *msg = FORMAT_MSG("shell", READ_ERROR_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      continue;
    }

    // Keep asking for inputs if user inputs nothing
    while (*buffer == '\n') {
      bytes_read = getUserInput(buffer);
    }

    // Handle getcwd() failure in getUserInput()
    if (bytes_read == 0) {
      break;
    }

    // Waitpid looks at all children and exits them WNOHANG
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }; // loops until no more exited children

    // remove newline character
    buffer[bytes_read] = '\0';
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
      // replace newline with null
      buffer[bytes_read - 1] = '\0';
      bytes_read--;
    }

    // Create array to store arguments
    char *arguments[20];
    int arg_count = 0;

    // Now, like in lab 2, use strtok_r to parse input
    char *saveptr = NULL;
    char *token = strtok_r(buffer, " ", &saveptr);
    while (token != NULL && arg_count < 19) {
      // Add the word/argument to the array
      arguments[arg_count++] = token;
      token = strtok_r(NULL, " ", &saveptr);
    }
    // Not sure if necessary, but make sure the array ends in NULL
    arguments[arg_count] = NULL;

    runProcess(arguments, arg_count);
  }
}
