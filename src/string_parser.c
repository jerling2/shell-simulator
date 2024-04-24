/*
 * 	string_parser.c
 *
 *  Author: Joseph Erlinger
 * 		Date: April 7, 2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"
#define _GUN_SOURCE


int count_token (char* buf, const char* delim)
{	
	int num_token;
	int state;
	int i;
	
	// Tokens start at 1 to make space for the NULL argument.
	num_token = 1;
	state = 0;
	i = 0;
	
	// Replace any newline characters with NULL.
	// WARNING: Potential risk of truncating strings.
	strtok (buf, "\n");

	// Skip all leading delimeters (if any).
	while (buf[i] == delim[0])
	{ 
		i ++;
	}
	
	// Increment tokens if sees a non delimiter after a delimiter.
	for (i; buf[i] != '\0'; i++) 
	{
		if (state == 0 && buf[i] != delim[0])
		{
			state = 1;
			num_token ++;
		}
		if (state == 1 && buf[i] == delim[0])
		{
			state = 0;
		}
	}

	return num_token;
}


command_line str_filler (char* buf, const char* delim)
{
	command_line command;
	char *saveptr;
	char *token;
	int i;

	i = 0;
	command.num_token = count_token (buf, delim);
	command.command_list = (char**) malloc (sizeof(char*) * command.num_token);

	token = strtok (buf, delim);
	while (token != NULL)
	{
		// IMPORTANT: duplicate token string.
		// Otherwise, multiple ptrs will point to same token.
		command.command_list[i] = strdup (token);
		token = strtok (NULL, delim);
		i ++;
	}
	// Make last token in the command list NULL (Lab requirement). 
	command.command_list[i] = NULL;
	return command;
}


void free_command_line (command_line* command)
{
	int i;

	// Free each token in the command list.
	for (i = 0; i < command->num_token - 1; i++)
	{
		free (command->command_list[i]);
	}

	// Free the command list.
	free (command->command_list); 
}
