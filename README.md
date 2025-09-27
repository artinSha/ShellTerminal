# Simple Shell In Linux

A UNIX-like shell implementation in C that provides command execution, built-in commands, and command history functionality.

## Features

### Core Functionality
- **Command Execution**: Execute external programs in foreground or background processes
- **Background Processes**: Use `&` at the end of commands to run in background
- **Built-in Commands**: Internal shell commands for common operations
- **Command History**: Access and re-run up to 10 most recent commands
- **Signal Handling**: Proper handling of SIGINT (Ctrl+C) without terminating the shell

### Built-in Commands

- **`exit`**: Exit the shell program
- **`pwd`**: Display current working directory
- **`cd [directory]`**: Change current directory
  - `cd` or `cd ~`: Change to home directory
  - `cd -`: Change to previous directory
  - `cd ~/path`: Support for tilde expansion
- **`help [command]`**: Display help information
  - `help`: Show all internal commands
  - `help <command>`: Show help for specific command
- **`history`**: Display the 10 most recent commands
- **`!n`**: Execute command number n from history
- **`!!`**: Execute the last command

## Project Structure

```
a8/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── msgs.h             # Error message macros (provided)
│   └── *.h                # Custom header files
├── src/                   # Source files
│   └── *.c                # Implementation files
└── gtest/                 # Test cases
    └── *.cpp              # Google Test files
```

## Building the Project

### Prerequisites
- CMake (3.10 or higher)
- Clang compiler
- Google Test framework

### Build Instructions

1. Set up the environment (if not already configured):
   ```bash
   export CC=$(which clang)
   export CXX=$(which clang++)
   ```

2. Generate build files:
   ```bash
   cmake -S . -B build
   ```

3. Compile the project:
   ```bash
   cmake --build build
   ```

This will generate:
- `build/shell`: The main shell executable
- `build/test_runner`: Test executable for running test cases

## Running the Shell

Execute the shell from the build directory:

```bash
./build/shell
```

### Example Usage

```bash
/home/user$ ls -la
# Lists files in current directory

/home/user$ echo "Hello World" &
# Runs echo command in background

/home/user$ cd /tmp
/tmp$ pwd
/tmp

/tmp$ history
2	pwd
1	cd /tmp
0	ls -la

/tmp$ !0
ls -la
# Re-runs the ls -la command

/tmp$ !!
ls -la
# Re-runs the last command

/tmp$ exit
```

## Testing

Run the test suite to verify functionality:

```bash
./build/test_runner
```

The tests cover:
- Basic command execution
- Background process handling
- Built-in command functionality
- History feature operations
- Signal handling
- Error conditions

## Compatibility

This shell is designed for UNIX-like systems and has been tested on:
- Linux distributions
- macOS (with appropriate development tools)

Requires POSIX-compliant system calls and standard C library functions.
