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
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *cmd, *cmd_token;
    char *pipe_token_ptr = NULL;
    char *whitespace_token_ptr = NULL;
    int num_pipes = 0;
    clist->num = 0;

    //check if cmd_line is empty
    if (cmd_line == NULL || strlen(cmd_line) == 0) {
        return OK;
    }

    //count number of pipes and validate
    int cmd_line_len = strlen(cmd_line);
    for (int i = 0; i < cmd_line_len; i++) {
        if (cmd_line[i] == PIPE_CHAR) {
            num_pipes++;
        }
    }
    if (num_pipes >= CMD_MAX) { 
        return ERR_TOO_MANY_COMMANDS;
    }

    cmd = strtok_r(cmd_line, PIPE_STRING, &pipe_token_ptr);
    while (cmd != NULL) {
        //trim leading and trailing whitespace
        while (isspace((unsigned char)*cmd)) {
            cmd++;
        }

        char *end = cmd + strlen(cmd) - 1;
        while (end > cmd && isspace((unsigned char)*end)) {
            end--;
        }
        end[1] = '\0';

        //separate executable and arguments
        cmd_token = strtok_r(cmd, " ", &whitespace_token_ptr);

        if (cmd_token == NULL) {
            cmd = strtok_r(NULL, PIPE_STRING, &pipe_token_ptr);
            continue;
        }

        //check executable and if valid, store in structure
        if (strlen(cmd_token) > EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        strcpy(clist->commands[clist->num].exe, cmd_token);

        //check arguments and if valid, store in structure
        int arg_index = 0;
        clist->commands[clist->num].args[0] = '\0';

        while ((cmd_token = strtok_r(NULL, " ", &whitespace_token_ptr)) != NULL) {
            if (strlen(clist->commands[clist->num].args) + strlen(cmd_token) + 2 > ARG_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            if (arg_index > 0) {
                strlcat(clist->commands[clist->num].args, " ", ARG_MAX);
            }
            strlcat(clist->commands[clist->num].args, cmd_token, ARG_MAX);
            arg_index++;
        }

        clist->num++;
        cmd = strtok_r(NULL, PIPE_STRING, &pipe_token_ptr);
    }
    return OK;
}
