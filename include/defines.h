#ifndef DEFINES_H
#define DEFINES_H

#define SERVER_FIFO_PATH "src/fifo_server"
#define CLIENT_FIFO_PATH "src/fifo_client"
#define SERVER "src/fifo_server"
#define CLIENT "src/fifo_client"

#define MAX_TASKS 3
#define MAX_PROGRAMS 10
#define MAX_PROGRAM_LENGTH 300
#define MAX_ARGUMENTS 10
#define MAX_ARGUMENT_LENGTH 50

typedef struct msg{
    int pid;
    int time;
    int option;
    char message[300];
} *Msg;

#endif