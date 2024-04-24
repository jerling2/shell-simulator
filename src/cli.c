/*
 *  cli.c
 *
 *  Author: Joseph Erlinger
 *      Created on: April 8, 2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include "string_parser.h"
#include "cli.h"
#include "command.h"
#define _GNU_SOURCE


#define STDOUT 1
#define STDERR 2
const char* OPERATOR_LIST[] = {"ls", "pwd", "mkdir", "cd", "cp", "mv", "rm",
                               "cat", "exit"};


/* Note: Only call from main. File mode reads from a batch file, and executes
each line as a command or sequence of commands. */
void file_mode (char *filename)
{
    FILE *stream;           // Stream of the input batch file.
    char *line_buf;         // Line buffer for storing a line from the stream.
    size_t len;             // Length of the line buffer.
    ssize_t nread;          // # Bytes read from stdin.
    SHELL_STATUS opcode;    // Status of the shell.

    line_buf = NULL;
    len = 0;
    opcode = RUNNING;

    /* Open the stream */
    if ((stream = fopen(filename, "r")) == NULL) {
        write(STDERR, "Error! File '", 13);
        write(STDERR, filename, strlen(filename));
        write(STDERR, "' not found.\n", 13);
        return;                         // Error! Could not read from filename.
    }

    /* Execute each line of commands from the stream. */
    while (opcode == RUNNING || opcode == ERROR) {
        if ((nread = getline(&line_buf, &len, stream)) == -1) {                              
            break;                                              // Reached EOF.
        }
        if (errno != 0) {
            print_syserr(errno, __func__);                            // DEBUG.             
            break;                        // Encounted some error in getline().
        }
        opcode = command_line_interface(line_buf);
    }

    /* Free resources and close the input stream. */
    fclose(stream);
    free(line_buf);
}   /* file_mode */


/* Note: Only call from main. Interactive mode prompts the user '>>> ' to input
a command or sequence of commands. Then, interactive mode gets the text from 
stdin and executes the command(s). */
void interactive_mode ()
{   
    char *line_buf;         // Line buffer for storing line from stdin.
    size_t len;             // Length of the line_buf.
    ssize_t nread;          // # Bytes read from stdin.
    SHELL_STATUS opcode;    // Status of the shell.
    
    line_buf = NULL;
    len = 0;
    opcode = RUNNING;
    
    while (opcode == RUNNING || opcode == ERROR) {
        write(STDOUT, ">>> ", 4);
        if ((nread = getline(&line_buf, &len, stdin)) == -1) {  
            write(STDOUT, "\n", 1);                              
            break;                                              // Reached EOF.
        }
        if (errno != 0) {
            print_syserr(errno, __func__);                            // DEBUG.             
            break;                        // Encounted some error in getline().
        }
        opcode = command_line_interface(line_buf);
    }
    free(line_buf);
}   /* interactive_mode */


/* The command line interface takes a single line of text and splits it into
command_line(s). It then gives each command_line to the command interpreter. */
SHELL_STATUS command_line_interface (char *buf)
{
    SHELL_STATUS opcode = RUNNING;
    command_line large_token_buffer;
    command_line small_token_buffer;

    large_token_buffer = str_filler(buf, ";");
    
    /* Iterate through each large token */
    for (int i = 0; large_token_buffer.command_list[i] != NULL; i++)
    {
        char* large_token = large_token_buffer.command_list[i];
        small_token_buffer = str_filler(large_token, " ");
        opcode = command_interpreter(small_token_buffer);        
        
        /* Free small token buffer and reset variables. */
        free_command_line(&small_token_buffer);
        memset(&small_token_buffer, 0, 0);
        
        /* stop processing the rest of the line on exit or error. */
        if (opcode == HALTED || opcode == ERROR)
            break;
    }
    /* Free large token buffer and reset variables. */
    free_command_line(&large_token_buffer);
    memset(&large_token_buffer, 0, 0);
    return opcode;
}   /* command_line_interface */


/* The command interpreter uses the tokens from a command_line to execute a
shell command. The command interpreter uses execute_command() to execute
the tokenized command line, and to display any errors. */
SHELL_STATUS command_interpreter (command_line command)
{
    shellCommand cmd;  // Datatype for passing into a try-catch function.
    char *op;          // The name of the command.
    OPTYPE op_type;    // The mapping of the command name to a enum.
    char **args;       // Alias for command.command_list
    int num_args;      // The number of operands - aka. # tokens - 1.
    int i;

    /* Initialize Variables */
    op = command.command_list[0];
    args = command.command_list;
    num_args = command.num_token - 2;
    op_type = DNE;

    /* Skip the NULL/empty/nothing comamnd */
    if (op == NULL || strcmp(op, "\n") == 0)
        return RUNNING;
   
    /* Match the operator to an OpType, then set op_type. */
    for (i = 0; i < OPERATOR_LIST_SIZE; i++) {
        if (strcmp(op, OPERATOR_LIST[i]) == 0) {
            op_type = i;
            break;
        }
    }

    /* Handling all the different shell commands */
    switch (op_type) {
        case LS:
            cmd.n = 0;
            cmd.fun.noInput = listDir;
            execute_command(cmd, args, num_args, "ls");
            break;
        case PWD:
            cmd.n = 0;
            cmd.fun.noInput = showCurrentDir;
            execute_command(cmd, args, num_args, "pwd");
            break;
        case MKDIR:
            cmd.n = 1;
            cmd.fun.oneInput = makeDir;
            execute_command(cmd, args, num_args, "mkdir");
            break;
        case CD:
            cmd.n = 1;
            cmd.fun.oneInput = changeDir;
            execute_command(cmd, args, num_args, "cd");
            break;
        case CP:
            cmd.n = 2;
            cmd.fun.twoInput = copyFile;
            execute_command(cmd, args, num_args, "cp");
            break;
        case MV:
            cmd.n = 2;
            cmd.fun.twoInput = moveFile;
            execute_command(cmd, args, num_args, "mv");
            break;
        case RM:
            cmd.n = 1;
            cmd.fun.oneInput = deleteFile;
            execute_command(cmd, args, num_args, "rm");
            break;
        case CAT:
            cmd.n = 1;
            cmd.fun.oneInput = displayFile;
            execute_command(cmd, args, num_args, "cat");
            break;
        case EXIT:
            return HALTED;
        case DNE:
            print_err(CMD, op);
            return ERROR;
    }
    if (errno != 0) {                      // Don't read rest of line if error.
        errno = 0;                                              // Reset errno.
        return ERROR;
    } else {
        return RUNNING;
    }
}   /* command_interpreter */


/* Generalized function for executing shell commands and catching any errors */
void execute_command (shellCommand cmd, char **args, int num_args, char *name)
{
    if (num_args != cmd.n) {
        print_err(PARAM, name);
        errno = EINVAL;        // Indirectly stop reading the rest of the line.
        return;                        // Error! Incorrect number of arguments.
    }
    if (num_args == 0) {
        cmd.fun.noInput();
    } else if (num_args == 1) {
        cmd.fun.oneInput(args[1]);
    } else if (num_args == 2) {
        cmd.fun.twoInput(args[1], args[2]);
    }
    if (errno != 0) {          // Check if something went wrong in the syscall.
        print_err(PARAM, name);
    }
}   /* execute_command */


/* FOR DEBUGGING: for problems outside of the command_interpreter. */
void print_syserr (int error_number, const char *error_msg)
{
    const char* error_str;    // The text description of an errno.
    
    error_str = strerror(error_number);
    write(STDERR, "Error! ", 7);
    write(STDERR, error_str, strlen(error_str));
    write(STDERR, ": ", 2);
    write(STDERR, error_msg, strlen(error_msg));
    write(STDERR, "\n", 1);
}   /* print_syserr */


/* Print a (rather unhelpful) error message if anything goes wrong. */
void print_err (ERR_TYPE err_type, const char *error_msg)
{
    write(STDOUT, "Error! ", 7);
    switch (err_type) {
        case CMD:
            write(1, "Unrecognized command: ", 22);
            break;
        case PARAM:
            write(1, "Unsupported parameters for command: ", 36);
            break;
    }
    write(STDOUT, error_msg, strlen(error_msg));
    write(STDOUT, "\n", 1);
}   /* print_err */
 