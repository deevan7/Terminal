# Flash Shell

Team Members:
- Don Das 
- Deevankumar Gaddala 

## Contribution 

### Logic and Code Writing (Don Das and Deevankumar Gaddala)

- **Don**: Implemented functionalities like `cmd_execution`, handling input/output redirection (`infile_outfile`), and parsing command-line options (`option_array`) and also the `main` function.

- **Deevankumar**: Focused on background process execution (`background`), and functions related to environment variables (`set_fn_parser`, `get_fn_parser`,`set_env_variable`,`get_env_var`).

### Testing (Daksh Bhandari)

- **Daksh**: Handled testing the entire shell functionality, ensuring it works as intended. He was also responsible for error handling.



### About the Project

Flash Shell is a simple Unix/Linux shell implemented in C. It provides basic functionalities like executing commands, setting and getting environment variables, and handling input/output redirection. Flash Shell also supports pipes for command chaining. Unless given the exit command, the shell is in what is called as the read-eval-print-loop (REPL). It reads a command, evaluates, i.e. runs, it, prints any results, and loops back to reading the next command.

## Features

1. Simple Command line
2. Background Processes
3. Sequence
4. Pipe (piping upto two processes)
5. Redirection of Input and Output.
6. Collects the return value of the commands it executes and also faithfully passes arguments specified by the user.
7. Setting and getting the Environament Variables.

## About the Code

### Global Variables

- `wstatus`: This variable stores the exit status of the last executed child process.
- `last_exit_status`: This variable maintains the exit status returned by the most recently executed command.
- `environment_variables`: This is an array of character pointers that holds the environment variables and their corresponding values. It's initialized with some predefined variables like `FOO` and `USER`.

### Functions

- `set_env_variable`: Updates an existing environment variable or creates a new one if it doesn't exist.
- `get_env_var`: Retrieves the value of an environment variable.
- `option_array`: Parses a string containing command-line options and stores them in an array.
- `set_fn_parser`: Parses a "set" command to extract the variable name and value.
- `get_fn_parser`: Parses a "get" command to extract the variable name.
- `infile_outfile`: Parses a command string for input/output redirection (`<` and `>`) and handles file opening.
- `cmd_execution`: The core function that executes commands. It handles background execution, pipes, redirection, special commands (`set`, `get`, `exit`), and forks child processes.

### Usage:

Compile the program using a C compiler:

```
gcc -o flash flash.c
```
or 
```
make
```

## Example usages

- Simple Commands
```
 flash$ ls -l   
```
- Background Process
```
 flash$ ls#
```
- Piping using `|`
```
 flash$ ls -l | wc -l
```
- Commands in sequence using `,`
```
 flash$ ls -l, echo "Hello World"
```
- Redirection of output using `>`
```
 flash$ ls -l > output.txt
```
- Redirecton of input using `<`
```
- flash$ wc -l < input.txt
```
- Setting of Environment Variables
```
- flash$ set LANG = "ENG"
```
- Printing the Value of environment variables
```
- flash$ get ?
```
- Exiting the shell
```
- flash$ exit
```