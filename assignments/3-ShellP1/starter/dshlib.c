#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
	memset(clist, 0, sizeof(command_list_t));
	if(strlen(cmd_line) == 0) {
		return WARN_NO_CMDS;
	}

	char pipe_copy[SH_CMD_MAX];
	strcpy(pipe_copy, cmd_line);

	char *saveptr1;
	char *cmd_string = strtok_r(pipe_copy, PIPE_STRING, &saveptr1);
	while(cmd_string != NULL) {
		if(clist->num >= CMD_MAX) {
			return ERR_TOO_MANY_COMMANDS;
		}

		char cmd_copy[SH_CMD_MAX];
		char *cmd_ptr = cmd_copy;
		strcpy(cmd_copy, cmd_string);

		// leading space skipping
		while(isspace(*cmd_ptr)) cmd_ptr++;

		// trailing space skipping
		char *end = cmd_ptr + strlen(cmd_ptr) - 1;
		while(end > cmd_ptr && isspace(*end)) {
			*end = '\0';
			end--;
		}

		if(strlen(cmd_ptr) > 0) {
			char *saveptr2;
			char *token = strtok_r(cmd_ptr, " ", &saveptr2);
			if(token) {
				if(strlen(token) >= EXE_MAX) {
					return ERR_CMD_OR_ARGS_TOO_BIG;
				}
				strcpy(clist->commands[clist->num].exe, token);
				token = strtok_r(NULL, "", &saveptr2);
				if(token) {
					while(isspace(*token)) token++;
					if(strlen(token) >= ARG_MAX) {
						return ERR_CMD_OR_ARGS_TOO_BIG;
					}
					strcpy(clist->commands[clist->num].args, token);
				}
				clist->num++;
			}
		}

		cmd_string = strtok_r(NULL, PIPE_STRING, &saveptr1); // next command
	}

	if(clist->num == 0) {
		return WARN_NO_CMDS;
	}

	return OK;
}
