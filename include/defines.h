#ifndef DEFINES_H
#define DEFINES_H

#define SERVER "src/fifo_server"
#define CLIENT "src/fifo_client"

#define COMPLETE "completed.txt" 

#define MAX_TASKS 3
#define MAX_PROGRAMS 10
#define MAX_PROGRAM_LENGTH 300
#define MAX_ARGUMENTS 10
#define MAX_ARGUMENT_LENGTH 50

typedef struct msg{
    int pid;
    int time;
    int status; // 0 -> in queue, 1 -> execution, 2 -> finished 
    int option; // 0 -> status, 1-> execute -u, 2 -> execute -p, 3 -> end
    char message[300];
} *Msg;

#endif