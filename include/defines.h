#ifndef DEFINES_H
#define DEFINES_H

#define SERVER "src/fifo_server"
#define CLIENT "src/fifo_client"

#define COMPLETE "completed.txt" 
#define EXECUTING "executing.txt" 
#define QUEUE "queue.txt" 


typedef struct msg{
    int pid;
    int time;
    int status; // 0 -> in queue, 1 -> execution, 2 -> finished 
    int option; // 0 -> status, 1-> execute -u, 2 -> execute -p, 3 -> end
    char message[300];
} *Msg;

#endif