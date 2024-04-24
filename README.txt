# CS 415 - Operating Systems - Project 1 

## Description

Pseudo-Shell is a UNIX-like shell programmed in C. There are two modes of operation: interactive mode and file mode. Run the shell in interactive mode with `./pseudo-shell` and file mode with `./pseudo-shell -f <filename>`. In interactive mode, the shell will prompt `>>>` and accept a sequence of commands delimited by `;`. The output of each command (if any) is displayed in the terminal. In file mode, the shell will execute a batch file containing lines of command sequences. The output of each command is redirected to a file named `output.txt` that is located in the project's root directory. The special `exit` command will close the Pseudo-Shell.

## Available Commands

Below is a list of available commands. If a command is not recognized, the shell will output `Error! Unrecognized command: <command>`. Similarly, if a command was given incorrect parameters, the shell will output `Error! Unsupported parameters for command: <command>`. The Pseudo-Shell will close on the `exit` command.

Note: the `cp` and `mv` commands are for copying and moving files only.

* `ls`
* `pwd`
* `mkdir <directory>`
* `cd <directory>`
* `cp <filename 1> <filename 2>`
* `mv <filename 1> <filename 2>`
* `rm <filename>`
* `cat <filename>`
* `exit`

## Environment

This project was designed in a Debian (xfce) environment.

## Installation

In the project's root directory, run `make`.

```bash
make
```

## Usage

```bash
./pseudo-shell [-f filename]
```

## Author

Joseph Erlinger

## Credit

Monil Guann for `string_parser.h` and Jared Hall for `command.h`.