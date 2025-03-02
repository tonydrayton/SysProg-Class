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

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
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

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char *p;
    int i = 0;
    bool in_quote = false;

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    p = cmd_buff->_cmd_buffer;

    while (*p == ' ') p++;
    if (!*p) return OK;

    cmd_buff->argv[0] = p;

    while (*p) {
        if (*p == '"') {
            if (!in_quote) {
                memmove(p, p + 1, strlen(p));
                in_quote = true;
            } else {
                memmove(p, p + 1, strlen(p));
                in_quote = false;
                if (*p == ' ') {
                    *p = '\0';
                    p++;
                    while (*p == ' ') p++;
                    if (*p && i < CMD_ARGV_MAX - 1) {
                        i++;
                        cmd_buff->argv[i] = p;
                    }
                    continue;
                }
            }
            continue;
        }

        if (*p == ' ' && !in_quote) {
            *p = '\0';
            p++;
            while (*p == ' ') p++;
            if (*p && i < CMD_ARGV_MAX - 1) {
                i++;
                cmd_buff->argv[i] = p;
            }
        } else {
            p++;
        }
    }

    cmd_buff->argv[i + 1] = NULL;
    cmd_buff->argc = i + 1;
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
        case BI_CMD_DRAGON:
            printf("                 /           /\n");
            printf("                /' .,,,,  ./\n");
            printf("               /';'     ,/\n");
            printf("              / /   ,,//,`'`\n");
            printf("             ( ,, '_,  ,,,' ``\n");
            printf("             |    /@  ,,, ;\" `\n");
            printf("            /    .   ,''/' `,``\n");
            printf("           /   .     ./, `,, ` ;\n");
            printf("        ,./  .   ,-,',` ,,/'\\,',\n");
            printf("       |   /; ./,,'`,,'' |   |  |\n");
            printf("       |     /   ','    /    |\n");
            printf("        \\___/'   '     |     |\n");
            printf("          `,,'  |      /     `\\\n");
            printf("               /      |        ~\\\n");
            printf("              '       (\n");
            printf("             :                   '\\\n");
            printf("            ; .         \\--       `\\\n");
            printf("          :   \\      `,'  \\,-       \\\n");
            printf("         .     .     /`    \\)         \\\n");
            printf("        .   ,  .    '  ,-'\"           \\\n");
            printf("        :   .  \\``/  ,'               `\\\n");
            printf("        '   ;   \\,'  /                  `,\n");
            printf("         \\  /    ;  /                    `,\n");
            printf("          `.    /` /                      `\\\n");
            printf("           |`. | `(                        `\\\n");
            printf("           |  \\`-`,;                        )\n");
            printf("         ,-|    |  |                        |\n");
            printf("        (,-'    | /                         |\n");
            printf("         |      |/                          |\n");
            printf("         |      |                           |\n");
            printf("         |      |                           |\n");
            return BI_EXECUTED;
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

int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return ERR_EXEC_CMD;
        }
    }
}

int execute_pipeline(command_list_t *clist) {
    int pipes[CMD_MAX-1][2];
    pid_t pids[CMD_MAX];

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int last_status = 0;
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        if (i == clist->num - 1 && WIFEXITED(status)) {
            last_status = WEXITSTATUS(status);
        }
    }

    return last_status;
}
