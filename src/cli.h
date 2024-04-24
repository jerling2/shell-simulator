/*
 *  cli.h
 *
 *  Author: Joseph Erlinger
 *      Created on: April 8, 2024
 */
#ifndef CLI_H
#define CLI_H

typedef enum OPTYPE {
    LS,
    PWD,
    MKDIR,
    CD,
    CP,
    MV,
    RM,
    CAT,
    EXIT,
    DNE
} OPTYPE;

extern const char* OPERATOR_LIST[];

#define OPERATOR_LIST_SIZE 9


typedef enum SHELL_STATUS {
    HALTED,
    RUNNING,
    ERROR
} SHELL_STATUS;


typedef enum ERR_TYPE {
    CMD,
    PARAM
} ERR_TYPE;


void interactive_mode ();


void file_mode ();


SHELL_STATUS command_line_interface (char *buf);


SHELL_STATUS command_interpreter (command_line command);


void print_syserr (int error_number, const char *error_msg);


void print_err (ERR_TYPE err_type, const char *error_msg);

typedef struct shellCommand {
    int n;
    union {
        void (*noInput)();
        void (*oneInput)(char*);
        void (*twoInput)(char*, char*);
    } fun;
} shellCommand;

void execute_command (shellCommand cmd, char **args, int num_args, char *name);

#endif  /* CLI_H */