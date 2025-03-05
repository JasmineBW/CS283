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
        printf("exiting...\n");
        return OK_EXIT;
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
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = malloc(ARG_MAX);
        if (cmd_buff->argv[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(cmd_buff->argv[j]);
            }
            free(cmd_buff->_cmd_buffer);
            return ERR_MEMORY;
        }
    }
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_output = 0;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        free(cmd_buff->argv[i]);
    }
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }
    if (cmd_buff->input_file != NULL) {
        free(cmd_buff->input_file);
    }
    if (cmd_buff->output_file != NULL) {
        free(cmd_buff->output_file);
    }
    if (cmd_buff->append_output != 0) {
        cmd_buff->append_output = 0;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        cmd_buff->argv[i] = NULL;
    }
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_output = 0;
    return OK;
}

int open_input_file(char *fname){
    int fd;
    int flags = O_RDONLY;

    fd = open(fname, flags);
    if (fd < 0){
        perror("input file open error");
        return fd;
    }
    return fd;
}

int open_output_file(char *fname){
    int fd;
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(fname, flags, mode);
    if (fd < 0){
        perror("appending file open error");
        return fd;
    }

    return fd;
}

int append_output_file(char *fname){
    int fd;
    int flags = O_WRONLY | O_CREAT | O_APPEND;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(fname, flags, mode);
    if (fd < 0){
        perror("output file open error");
        return fd;
    }

    return fd;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char token[ARG_MAX];
    char file_name[ARG_MAX];
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
        } 
        else if (*ptr == REDIR_CHAR_IN || *ptr == REDIR_CHAR_OUT || strncmp(ptr, REDIR_STRING_OUT_APPEND, 2) == 0) {
            char redir_char;
            int file_length = 0;

            if (strncmp(ptr, REDIR_STRING_OUT_APPEND, 2) == 0) {
                cmd_buff->append_output = 1;
                ptr += 2;
            } else {
                redir_char = *ptr;
                ptr++;
            }

            while (*ptr != '\0' && isspace((unsigned char)*ptr)) {
                ptr++;
            }

            while (*ptr != '\0'  && *ptr != PIPE_CHAR && !isspace((unsigned char)*ptr)) {
                if (file_length >= ARG_MAX - 1) {
                    free(temp_string);
                    free_cmd_buff(cmd_buff);
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }
                file_name[file_length++] = *ptr;
                ptr++;
            }
            file_name[file_length] = '\0';

            if (redir_char == REDIR_CHAR_IN) {
                cmd_buff->input_file = malloc(strlen(file_name) + 1);
                if (cmd_buff->input_file == NULL) {
                    free(temp_string);
                    free_cmd_buff(cmd_buff);
                    return ERR_MEMORY;
                }
                strcpy(cmd_buff->input_file, file_name);
            } else {
                cmd_buff->output_file = malloc(strlen(file_name) + 1);
                if (cmd_buff->output_file == NULL) {
                    free(temp_string);
                    free_cmd_buff(cmd_buff);
                    return ERR_MEMORY;
                }
                strcpy(cmd_buff->output_file, file_name);
            }
            continue;
        }
        else {
            while (*ptr != '\0' && !isspace((unsigned char)*ptr)) {
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
    
    free(temp_string);
    return OK;
}

cmd_buff_t clone_command_buffer(cmd_buff_t *src) {
    cmd_buff_t copyBuffer;
    copyBuffer.argc = src->argc;
    copyBuffer._cmd_buffer = strdup(src->_cmd_buffer);
    for (int i = 0; i < (src->argc); i++) {
        copyBuffer.argv[i] = (src->argv[i] != NULL) ? strdup(src->argv[i]) : NULL;
    }
    for (int j = (src->argc); j < CMD_ARGV_MAX; j++) {
        copyBuffer.argv[j] = NULL;
    }
    copyBuffer.input_file = (src->input_file != NULL) ? strdup(src->input_file) : NULL;
    copyBuffer.output_file = (src->output_file != NULL) ? strdup(src->output_file) : NULL;
    return copyBuffer;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *cmd;
    char *pipe_token_ptr = NULL;
    int num_pipes = 0;
    clist->num = 0;

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
        //trim leading whitespace
        while (isspace((unsigned char)*cmd)) {
            cmd++;
        }

        //trim trailing whitespace
        char *end = cmd + strlen(cmd) - 1;
        while (end > cmd && isspace((unsigned char)*end)) {
            end--;
        }
        end[1] = '\0';

        cmd_buff_t *cmd_buff = malloc(sizeof(cmd_buff_t));
        if (cmd_buff == NULL) {
            return ERR_MEMORY;
        }
        if (alloc_cmd_buff(cmd_buff) != OK) {
            free(cmd_buff);
            return ERR_MEMORY;
        }

        if (build_cmd_buff(cmd, cmd_buff) != OK) {
            free_cmd_buff(cmd_buff);
            free(cmd_buff);
            return ERR_EXEC_CMD;
        }

        if (cmd_buff->argc == 0 || cmd_buff->_cmd_buffer == NULL) {
            free_cmd_buff(cmd_buff);
            free(cmd_buff);
            cmd = strtok_r(NULL, PIPE_STRING, &pipe_token_ptr);
            continue;
        }

        // Deep copy cmd_buff into the command list.
        clist->commands[clist->num] = clone_command_buffer(cmd_buff);
        clist->num++;

        free_cmd_buff(cmd_buff);
        free(cmd_buff);

        cmd = strtok_r(NULL, PIPE_STRING, &pipe_token_ptr);
        }
    return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff)
{
    for (int i = 0; i < CMD_ARGV_MAX; i++)
    {
        free(cmd_buff->argv[i]);
    }
    free(cmd_buff->_cmd_buffer);
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for(int i = 0; i<CMD_MAX; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
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

int execute_pipeline(command_list_t *clist) {
    int num_commands = clist->num;
    int pipes[num_commands-1][2];
    pid_t process_ids[num_commands];

    // Create all necessary pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++) {
        process_ids[i] = fork();
        if (process_ids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (process_ids[i] == 0) {  // Child process
            if (clist->commands[i].input_file != NULL) {
                int fd_input = open_input_file(clist->commands[i].input_file);
                if (fd_input == -1) {
                    perror("opening input file failed");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_input, STDIN_FILENO);
                close(fd_input);
            } else if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            if (clist->commands[i].output_file != NULL) {
                int fd_output = (clist->commands[i].append_output == 1) ? append_output_file(clist->commands[i].output_file) : open_output_file(clist->commands[i].output_file);
                if (fd_output == -1) {
                    perror("opening output file failed");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_output, STDOUT_FILENO);
                close(fd_output);
            } else if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            exec_cmd(&clist->commands[i]);
        }
    }
    // Parent process: close all pipe ends
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    int status;
    for (int i = 0; i < num_commands; i++) {
        waitpid(process_ids[i], &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            last_rc = WEXITSTATUS(status);
        } else {
            last_rc = OK;
        }
    }
    return last_rc;
}

int exec_cmd(cmd_buff_t *cmd_buff) {
    execvp(cmd_buff->argv[0], cmd_buff->argv);
    perror("execvp");
    exit(EXIT_FAILURE);
}

int exec_local_cmd_loop() {
    char *cmd_buff = (char *)malloc(SH_CMD_MAX);
    int rc = 0;
    command_list_t clist;

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

        // build cmd list
        rc = build_cmd_list(cmd_buff, &clist);

        if (rc != OK) {
            printf("%d\n", rc);
            last_rc = rc;
            if (rc == WARN_NO_CMDS) {
                printf(CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else if (rc == ERR_MEMORY || rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf(CMD_ERR_EXECUTE);
            }
            continue;
        }

        if ((rc = exec_built_in_cmd(&clist.commands[0])) != BI_NOT_BI) {
            if (rc != OK) {
                if (rc == OK_EXIT) {
                    last_rc = OK;
                    break;
                } else {
                    printf(CMD_ERR_EXECUTE);
                } 
            }
        } else {
            rc = execute_pipeline(&clist);
            if (rc != OK) {
                handle_error_code(rc);
            }
        }
    }
    return last_rc;
}

