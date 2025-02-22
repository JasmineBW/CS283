#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dragon.h"
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 *
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

 int last_rc;

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }
    else if (strcmp(input, DRAGON_CMD) == 0) {
        return BI_CMD_DRAGON;
    }
    else if (strncmp(input, CD_CMD, 2) == 0) {
        if (isspace(input[2]))
        {
            return BI_CMD_CD;
        }
        return BI_NOT_BI;
    }
    else if (strcmp(input, RC_CMD) == 0) {
        return BI_RC;
    }
    else {
        return BI_NOT_BI;
    }
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    switch (match_command(cmd->_cmd_buffer))
    {
    case BI_CMD_EXIT:
        exit(OK_EXIT);
        break;

    case BI_CMD_DRAGON:
        print_dragon();
        last_rc = OK;
        break;

    case BI_CMD_CD:
        if ((cmd->argv[1]) == NULL)
        {
            return OK;
        }
        if ((last_rc = chdir(cmd->argv[1])) == -1)
        {
            return ERR_EXEC_CMD;
        }
        return OK;

    case BI_RC:
        printf("%d\n", last_rc);
        last_rc = OK;
        return last_rc;

    default:
        return BI_NOT_BI;
    }
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL)
    {
        return ERR_MEMORY;
    }
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        cmd_buff->argv[i] = (char *)malloc(ARG_MAX);
        if (cmd_buff->argv[i] == NULL)
        {
            free(cmd_buff->argv[i]);
            return ERR_MEMORY;
        }
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        free(cmd_buff->argv[i]);
    }
    free(cmd_buff->_cmd_buffer);
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        cmd_buff->argv[i] = NULL;
    }
    cmd_buff->_cmd_buffer = NULL;
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char token[ARG_MAX];
    int token_length = 0;

    if (alloc_cmd_buff(cmd_buff) != OK) {
        return ERR_MEMORY;
    }

    //trim leading whitespace
    while (isspace((unsigned char)*cmd_line)) {
        cmd_line++;
    }

    //trim trailing whitespace
    char *end = cmd_line + strlen(cmd_line) - 1;
    while (end > cmd_line && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';

    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (cmd_buff->_cmd_buffer == NULL) {
        free_cmd_buff(cmd_buff);
        return ERR_MEMORY;
    }

    char *temp_string = strdup(cmd_line);
    if (temp_string == NULL) {
        free_cmd_buff(cmd_buff);
        return ERR_MEMORY;
    }
   
    char *ptr = temp_string;
    while (*ptr != '\0') {  
        while (*ptr != '\0' && isspace((unsigned char)*ptr)) {
            ptr++;
        }
        if (*ptr == '\0') {
            break;
        }
        
        token_length = 0;
        if (*ptr == '"') {
            ptr++;
            while (*ptr != '\0' && *ptr != '"') {
                if (token_length >= ARG_MAX - 1) {
                    free(temp_string);
                    free_cmd_buff(cmd_buff);
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }
                token[token_length++] = *ptr;
                ptr++;
            }
            if (*ptr != '"') {
                free(temp_string);
                free_cmd_buff(cmd_buff);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            ptr++;
        } else {
            while (*ptr != '\0' && !isspace((unsigned char)*ptr)) {  // corrected condition
                if (token_length >= ARG_MAX - 1) {
                    free(temp_string);
                    free_cmd_buff(cmd_buff);
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }
                token[token_length++] = *ptr;
                ptr++;
            }
        }
        token[token_length] = '\0';

        if (cmd_buff->argc < CMD_ARGV_MAX) {
            strcpy(cmd_buff->argv[cmd_buff->argc], token);
            cmd_buff->argc++;
        } else {
            free(temp_string);
            free_cmd_buff(cmd_buff);
            return ERR_TOO_MANY_COMMANDS;
        }
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    free(temp_string);
    return OK;
}

void handle_error_code(int rc) {
    switch (rc) {
        case ENOENT:
            printf(ERR_ENOENT);
            break;
        case EACCES:
            printf(ERR_EACCES);
            break;
        case ENOEXEC:
            printf(ERR_ENOEXEC);
            break;
        case E2BIG:
            printf(ERR_E2BIG);
            break;
        case ETXTBSY:
            printf(ERR_ETXTBSY);
            break;
        case ENOMEM:
            printf(ERR_ENOMEM);
            break;
        case EFAULT:
            printf(ERR_EFAULT);
            break;
        default:
        printf(CMD_ERR_EXECUTE);
            break;
    }
}

int exec_cmd(cmd_buff_t *cmd_buff) {
    pid_t pid = fork();

    if (pid < 0) {
        printf(CMD_ERR_EXECUTE);
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        if (execvp(cmd_buff->argv[0], cmd_buff->argv) == -1) {
            _exit(errno);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            last_rc = exit_status == 0 ? OK : exit_status;
            return last_rc;
        } else {
            last_rc = ERR_EXEC_CMD;
            return ERR_EXEC_CMD;
        }
    }
    last_rc = OK;
    return OK;
}

int exec_local_cmd_loop()
{
    char *cmd_buff = (char *)malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t cmd;

    while (1) {
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // check for empty command
        if (*cmd_buff == '\0') {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        // build cmd buffer
        rc = build_cmd_buff(cmd_buff, &cmd);

        if (cmd.argc == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (rc != OK) {
            if (rc == ERR_MEMORY || rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf(CMD_ERR_EXECUTE);
            }
            continue;
        }

        if (match_command(cmd._cmd_buffer) != BI_NOT_BI) {
            rc = exec_built_in_cmd(&cmd);
            if (rc != OK) {
                printf(CMD_ERR_EXECUTE);
            }
        } else {
            rc = exec_cmd(&cmd);
            if (rc != OK) {
                handle_error_code(rc);
            }
        }
    }
    return OK;
}