#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

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
int exec_local_cmd_loop()
{
    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    int last_return_code = 0;
    command_list_t cmd_list;

    if (!cmd_buff) {
        return ERR_MEMORY;
    }

    while(1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0) {
            continue;
        }

        if (cmd_buff[strlen(cmd_buff) - 1] == PIPE_CHAR) {
            printf("warning: trailing pipe with no command\n");
            continue;
        }

        rc = build_cmd_list(cmd_buff, &cmd_list);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                continue;
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                continue;
            }
            free_cmd_list(&cmd_list);
            continue;
        }

        Built_In_Cmds bi_cmd = exec_built_in_cmd(&cmd_list.commands[0]);
        if (bi_cmd == BI_CMD_EXIT) {
            free_cmd_list(&cmd_list);
            free(cmd_buff);
            return OK;
        } else if (bi_cmd == BI_EXECUTED) {
            free_cmd_list(&cmd_list);
            continue;
        }

        last_return_code = execute_pipeline(&cmd_list);

        free_cmd_list(&cmd_list);
    }

    free(cmd_buff);
    return OK;
}

int alloc_cmd_buff(cmd_buff_t *command_buffer) {
    command_buffer->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!command_buffer->_cmd_buffer) {
        return ERR_MEMORY;
    }
    command_buffer->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        command_buffer->argv[i] = NULL;
    }
    command_buffer->input_file = NULL;
    command_buffer->output_file = NULL;
    command_buffer->append_output = 0;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int build_cmd_buff(char *command_line, cmd_buff_t *command_buffer) {
    char *current_char;
    int arg_index = 0;
    bool in_quote = false;

    strncpy(command_buffer->_cmd_buffer, command_line, SH_CMD_MAX - 1);
    command_buffer->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    current_char = command_buffer->_cmd_buffer;

    command_buffer->input_file = NULL;
    command_buffer->output_file = NULL;
    command_buffer->append_output = 0;

    while (*current_char == ' ') current_char++;
    if (!*current_char) return OK;

    command_buffer->argv[0] = current_char;

    while (*current_char) {
        if (*current_char == '"') {
            if (!in_quote) {
                memmove(current_char, current_char + 1, strlen(current_char));
                in_quote = true;
            } else {
                memmove(current_char, current_char + 1, strlen(current_char));
                in_quote = false;
                if (*current_char == ' ') {
                    *current_char = '\0';
                    current_char++;
                    while (*current_char == ' ') current_char++;
                    if (*current_char && arg_index < CMD_ARGV_MAX - 1) {
                        arg_index++;
                        command_buffer->argv[arg_index] = current_char;
                    }
                    continue;
                }
            }
            continue;
        }

        if (!in_quote) {
            if (*current_char == '>' && *(current_char + 1) == '>') {
                *current_char = '\0';
                current_char += 2;

                while (*current_char == ' ') current_char++;

                if (*current_char) {
                    command_buffer->output_file = current_char;
                    command_buffer->append_output = 1;

                    while (*current_char && *current_char != ' ' && *current_char != '<' && *current_char != '>')
                        current_char++;

                    if (*current_char) {
                        *current_char = '\0';
                        current_char++;
                    }
                }
                continue;
            }
            else if (*current_char == '>') {
                *current_char = '\0';
                current_char++;

                while (*current_char == ' ') current_char++;

                if (*current_char) {
                    command_buffer->output_file = current_char;

                    while (*current_char && *current_char != ' ' && *current_char != '<' && *current_char != '>')
                        current_char++;

                    if (*current_char) {
                        *current_char = '\0';
                        current_char++;
                    }
                }
                continue;
            }
            else if (*current_char == '<') {
                *current_char = '\0';
                current_char++;

                while (*current_char == ' ') current_char++;

                if (*current_char) {
                    command_buffer->input_file = current_char;

                    while (*current_char && *current_char != ' ' && *current_char != '<' && *current_char != '>')
                        current_char++;

                    if (*current_char) {
                        *current_char = '\0';
                        current_char++;
                    }
                }
                continue;
            }
        }

        if (*current_char == ' ' && !in_quote) {
            *current_char = '\0';
            current_char++;
            while (*current_char == ' ') current_char++;
            if (*current_char && arg_index < CMD_ARGV_MAX - 1) {
                arg_index++;
                command_buffer->argv[arg_index] = current_char;
            }
        } else {
            current_char++;
        }
    }

    command_buffer->argv[arg_index + 1] = NULL;
    command_buffer->argc = arg_index + 1;
    return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff) {
    return free_cmd_buff(cmd_buff);
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token;
    char *saveptr;
    char *cmd_copy = strdup(cmd_line);
    int cmd_count = 0;
    bool empty_command_found = false;

    if (!cmd_copy) {
        return ERR_MEMORY;
    }

    clist->num = 0;

    for (int i = 0; i < strlen(cmd_line) - 1; i++) {
        if (cmd_line[i] == PIPE_CHAR && cmd_line[i+1] == PIPE_CHAR) {
            printf("warning: empty command in pipeline\n");
            free(cmd_copy);
            return WARN_NO_CMDS;
        }
    }

    token = strtok_r(cmd_copy, PIPE_STRING, &saveptr);

    while (token != NULL && cmd_count < CMD_MAX) {
        char *trimmed = token;
        while (*trimmed && isspace(*trimmed)) trimmed++;

        if (*trimmed == '\0') {
            printf("warning: empty command in pipeline\n");
            empty_command_found = true;
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if (alloc_cmd_buff(&clist->commands[cmd_count]) != OK) {
            free(cmd_copy);
            return ERR_MEMORY;
        }
        if (build_cmd_buff(token, &clist->commands[cmd_count]) != OK) {
            free(cmd_copy);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        if (clist->commands[cmd_count].argc == 0) {
            printf("warning: empty command in pipeline\n");
            free_cmd_buff(&clist->commands[cmd_count]);
            empty_command_found = true;
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }

        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    if (token != NULL && cmd_count >= CMD_MAX) {
        printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        free(cmd_copy);
        return ERR_TOO_MANY_COMMANDS;
    }

    clist->num = cmd_count;
    free(cmd_copy);

    if (cmd_count == 0) {
        printf(CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }
    if (cmd_line[strlen(cmd_line) - 1] == PIPE_CHAR) {
        printf("warning: trailing pipe with no command\n");
        return WARN_NO_CMDS;
    }
    if (empty_command_found) {
        return WARN_NO_CMDS;
    }

    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    cmd_lst->num = 0;
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    }
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bi_cmd = match_command(cmd->argv[0]);

    switch (bi_cmd) {
        case BI_CMD_EXIT:
            printf("exiting...\n");
            return BI_CMD_EXIT;
        case BI_CMD_CD:
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                }
            }
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}

int exec_cmd(cmd_buff_t *command) {
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (child_pid == 0) {
        if (command->input_file != NULL) {
            int input_fd = open(command->input_file, O_RDONLY);
            if (input_fd < 0) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (command->output_file != NULL) {
            int output_fd;
            if (command->append_output) {
                output_fd = open(command->output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                output_fd = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }

            if (output_fd < 0) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(command->argv[0], command->argv);

        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
    //parent
        int status;
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return ERR_EXEC_CMD;
        }
    }
}

int execute_pipeline(command_list_t *command_list) {
    int pipe_fds[CMD_MAX-1][2];
    pid_t child_pids[CMD_MAX];

    for (int i = 0; i < command_list->num - 1; i++) {
        if (pipe(pipe_fds[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    for (int cmd_index = 0; cmd_index < command_list->num; cmd_index++) {
        child_pids[cmd_index] = fork();

        if (child_pids[cmd_index] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (child_pids[cmd_index] == 0) {
            if (cmd_index > 0) {
                dup2(pipe_fds[cmd_index-1][0], STDIN_FILENO);
            }

            if (cmd_index < command_list->num - 1) {
                dup2(pipe_fds[cmd_index][1], STDOUT_FILENO);
            }

            if (cmd_index == 0 && command_list->commands[cmd_index].input_file != NULL) {
                int input_fd = open(command_list->commands[cmd_index].input_file, O_RDONLY);
                if (input_fd < 0) {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (cmd_index == command_list->num - 1 && command_list->commands[cmd_index].output_file != NULL) {
                int output_fd;
                if (command_list->commands[cmd_index].append_output) {
                    output_fd = open(command_list->commands[cmd_index].output_file,
                                    O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    output_fd = open(command_list->commands[cmd_index].output_file,
                                    O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                if (output_fd < 0) {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            for (int j = 0; j < command_list->num - 1; j++) {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }

            execvp(command_list->commands[cmd_index].argv[0],
                   command_list->commands[cmd_index].argv);

            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < command_list->num - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    int last_exit_status = 0;
    for (int i = 0; i < command_list->num; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);

        if (i == command_list->num - 1 && WIFEXITED(status)) {
            last_exit_status = WEXITSTATUS(status);
        }
    }

    return last_exit_status;
}
