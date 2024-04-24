/*
 * main.c
 *
 * Author: Joseph Erlinger 
 *      Created on: April 8, 2024    
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string_parser.h"
#include "cli.h"
#define _GNU_SOURCE


#define STDERR 2


int main(int argc, char *argv[])
{
    FILE *output_stream;    // Output stream for file mode.
    int flags, opt;        
    char *filename;         // The batch file for file mode.

    flags = 0;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            flags = 1;
            filename = optarg;     // Filename is the next arg after -f option.
            break;
        default: /* '?' */
            goto error;
        }
    }
 
    if (optind != argc || argc > 3)    // Only allow up to 3 args and 1 option.
        goto error;

    if (flags == 0) {
        interactive_mode();
    } else if (flags == 1) {
        output_stream = freopen("output.txt", "w", stdout);
        file_mode(filename);
        fclose(output_stream);
    }
    exit(EXIT_SUCCESS);

    error:
    write(STDERR, "Usuage: ", 8);
    write(STDERR, argv[0], strlen(argv[0]));
    write(STDERR, " [-f filename]\n", 16);
    exit(EXIT_FAILURE);
}   /* main */