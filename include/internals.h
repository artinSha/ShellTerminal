#ifndef INTERNALS_H
#define INTERNALS_H

#include <stdbool.h>

bool checkInternalCommands(char **arguments, int arg_count, bool background);

void changeDir(const char *path);

void displayAllHelp();

#endif
