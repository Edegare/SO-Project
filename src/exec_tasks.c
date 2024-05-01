#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>


void execute_command(char *command, int id, char *output_folder) {
    int pipes[16][2];
    int curr_pipe = 0;
    pid_t pid;
    char *args[300];
    char *token;
    char *rest = command;
    int i = 0;

    int output_fd, error_fd;

    //Output file
    char output_file[64];
    //Error file
    char error_file[64];
    snprintf(output_file, sizeof(output_file), "%s/TASK%d_output", output_folder, id);
    snprintf(error_file, sizeof(error_file), "%s/TASK%d_error", output_folder, id);
    
    // open error file
    error_fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (error_fd == -1) {
        perror("Error opening error file");
        _exit(1);
    }

    while ((token = strtok_r(rest, " ", &rest))) {
        if (i == 0) {
            args[0] = strdup(token);
            i++;
        }

        if (*token == '|') {
            if (pipe(pipes[curr_pipe])==-1){
                perror("Error creating pipe");
                close(error_fd);
                _exit(1);
            }
            else {
                args[i] = NULL;
                pid = fork();
                if (pid==-1) {
                    perror("Error doing fork");
                    close(error_fd);
                    _exit(1);
                }
                else if (pid  == 0) {
                    // will redirect output to respective pipes
                    close(pipes[curr_pipe][0]);
                    if (curr_pipe != 0) {
                        dup2(pipes[curr_pipe - 1][0], 0);
                        close(pipes[curr_pipe - 1][0]);
                    }
                    dup2(error_fd, 2);
                    close(error_fd);

                    dup2(pipes[curr_pipe][1], 1);
                    close(pipes[curr_pipe][1]);

                    execvp(args[0], args + 1);
                    perror("Error executing");
                    _exit(127);
                } else {
                    close(pipes[curr_pipe][1]);
                    if (curr_pipe != 0) {
                        close(pipes[curr_pipe - 1][0]);
                    }
                    curr_pipe++;
                }
            }
            i = 0; 
        } else {
            args[i] = strdup(token);
            i++;
        }
    }

    // Open output file
    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Error opening output file");
        for (int j = 0; j < i; j++) {
            if (args[j]!=NULL) free(args[j]);
        }
        close(error_fd);
        _exit(1);
    }

    args[i] = NULL;
    pid = fork();
    if (pid==-1){
        perror("Error doing fork");
        _exit(1);
    }
    else if (pid == 0) {
        if (curr_pipe != 0) {
            dup2(pipes[curr_pipe - 1][0], 0);
            close(pipes[curr_pipe - 1][0]);
        }
        dup2(error_fd, 2);
        close(error_fd);

        dup2(output_fd, 1);
        close(output_fd);

        execvp(args[0], args + 1);
        perror("Error executing");
        _exit(127);
    }

    close(error_fd);
    close(output_fd);

    for (int j = 0; j < i; j++) {
        if (args[j]!=NULL) free(args[j]);
    }
    for (int j = 0; j < curr_pipe + 1; j++) {
        wait(NULL);
    }

}