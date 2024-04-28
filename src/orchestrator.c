#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>


#include "defines.h"


int main (int argc, char * argv[]){
	

	int stop=0;
    int pipeQueue[2];

    if (pipe(pipeQueue)==-1) {
        perror ("pipe");
        return -1;
    }

	if (mkfifo(SERVER, 0666) == -1) {perror("Server: Error creating server FIFO");_exit(1);}
	

    int fd_sv, fd_cl;


	fd_sv = open (SERVER, O_RDONLY);
	if (fd_sv == -1) {
		perror("Server: Error opening server FIFO");
		_exit(1);
	}

    // == CHILD DOES TASKS ==
    for (int i=0; i<MAX_TASKS; i++) {
        int bytes_read_child;

            int fork_ret= fork();

            if (fork_ret == -1) {
                perror ("fork");
                i--;
            }

            else if (fork_ret==0) {
                Msg task= malloc(sizeof(struct msg));
                if (task==NULL) {
                    perror ("Server: malloc server message task");
                    _exit (127);
                }
                close(pipeQueue[1]);

                while (!stop){
                    
                    if ((bytes_read_child=read(pipeQueue[0],task,sizeof(struct msg))) > 0){
                        if (task->option==3) stop=1;
                        else {
                            //parse new->message to commands[][] array TODO before the rest 
                            printf("TASK %d DONE\n", task->pid);
                        }
                    }
                }
                close(pipeQueue[0]);
                _exit (0);
            }
    }

    // == DAD ==
	while (!stop) {

        int bytes_read=0;
        

		Msg new = malloc(sizeof(struct msg));
		if (new==NULL) {
			perror ("Server: malloc server message new");
			stop=1;
		}
        else if ((bytes_read=read(fd_sv,new,sizeof(struct msg))) > 0){

            // If client says to end, stop cycle
            if (new->option==3) {
                stop=1;
                for (int i=0;i<MAX_TASKS; i++){
                    if (write(pipeQueue[1], new,sizeof(struct msg)) == -1){
                        perror("Server: Error writing to pipeQueue");
                        int error=-1;
                        if (write(fd_cl, &error, sizeof(int)) == -1){
                            perror("Server: Error writing to client FIFO");
                        }
                    }
                }
            }
            else {
                if (new->option==0) {

                }
                else {

                    // tasks of execution to queue - Message client if received or not
                    char fifo[30];
                    sprintf (fifo,CLIENT"%d", new->pid);

                    fd_cl = open (fifo, O_WRONLY);
                    if (fd_cl == -1) {
                        perror("Server: Error opening client FIFO");
                    }
                    else {
                        // Put message in queue pipe and writes to client
                        
                        if (write(pipeQueue[1], new,sizeof(struct msg)) == -1){
                            perror("Server: Error writing to pipeQueue");
                            int error=-1;
                            if (write(fd_cl,&error, sizeof(int)) == -1){
                                perror("Server: Error writing to client FIFO");
                            }
                        }
                        else {
                            int id=new->pid;
                            //Send the number of task to client
                            if (write(fd_cl, &id, sizeof(int)) == -1){
                                perror("Server: Error writing to client FIFO");
                            }
                        }

                        close (fd_cl);
                    }
                }
                
            }
        }
		free (new);
	}

    close(pipeQueue[0]);
    close(pipeQueue[1]);

    // Wait for childs
	for (int j=0; j<MAX_TASKS; j++) {
		wait(NULL);
	}

	close(fd_sv);
	unlink(SERVER);
	

	return 0;
}

/* for (int i=0;i<parallel_tasks; i++) {
            pid_t who = waitpid(-1, NULL, WNOHANG);
            // If a pid finishes
            if (who > 0) {
                parallel_tasks--;
            } 
            // error
            else if (who == -1) {
                perror("Server: waitpid");
            }
            // else continue
        } */
/* while(parallel_tasks<MAX_TASKS && tasks_queue>0) {

                    Msg task= malloc(sizeof(struct msg));
                    if (task==NULL) {
                        perror ("Server: malloc server message task");
                    }
                    else {
                        parallel_tasks++;
                        tasks_queue--;
                        int fork_ret= fork();

                        if (fork_ret == -1) {
                            perror ("fork");
                            parallel_tasks--;
                        }

                        else if (fork_ret==0) {

                            close(pipeQueue[1]);
                            if ((bytes_read=read(pipeQueue[0],task,sizeof(struct msg))) > 0){
                                close(pipeQueue[0]);
                                //parse new->message to commands[][] array TODO before the rest 
                                //printf("TASK %d DONE\n", task->pid);
                            
                            
                            }
                            _exit (0);
                        }
                        
                    }
                } */