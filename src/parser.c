#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include "../include/defines.h"

void execute_command(char *command) {
    int pipes[32][2];
    int currPipe = 0;
    pid_t pid;
    char *args[1000];
    char *token;
    char *rest = command;
    int i = 0;

    while ((token = strtok_r(rest, " ", &rest))) {
        if (i == 0) {
            args[0] = strdup(token);
            i++;
        }

        if (*token == '|') {
            pipe(pipes[currPipe]);

            args[i] = NULL;
            if ((pid = fork()) == 0) {
                close(pipes[currPipe][0]);
                if (currPipe != 0) {
                    dup2(pipes[currPipe - 1][0], STDIN_FILENO);
                    close(pipes[currPipe - 1][0]);
                }
                dup2(pipes[currPipe][1], STDOUT_FILENO);
                close(pipes[currPipe][1]);
                execvp(args[0], args + 1);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                close(pipes[currPipe][1]);
                if (currPipe != 0) {
                    close(pipes[currPipe - 1][0]);
                }
                currPipe++;
            }
            i = 0; 
        } else {
            args[i] = strdup(token);
            i++;
        }
    }

    args[i] = NULL;
    if ((pid = fork()) == 0) {
        if (currPipe != 0) {
            dup2(pipes[currPipe - 1][0], STDIN_FILENO);
            close(pipes[currPipe - 1][0]);
        }
        execvp(args[0], args + 1);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}