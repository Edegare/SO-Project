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
	int parallel_tasks=0;
    int ntask=0;


	if (mkfifo(SERVER, 0666) == -1) {perror("Client: Error creating server FIFO");_exit(1);}
	

    int fd_sv, fd_cl, bytes_read;

	fd_sv = open (SERVER, O_RDONLY);
	if (fd_sv == -1) {
		perror("Server: Error opening server FIFO");
		_exit(1);
	}

	while (!stop) {

		Msg new = malloc(sizeof(struct msg));
		if (new==NULL) {
			perror ("Server: malloc server");
			stop=1;
			break;
		}

        else if (parallel_tasks<=MAX_TASKS) {
            if ((bytes_read=read(fd_sv,new,sizeof(struct msg))) > 0){
                // If client says to end, stop cycle
                if (new->option==3) {stop=1;}
                else {

                    ntask++;
                    int fork_ret= fork();

                    if (fork_ret == -1) {
                        perror ("fork");
                        ntask--;
                    }

                    else if (fork_ret==0) {

                        //parse new->message to commands[][] array TODO before the rest 

                        char fifo[30];
                        sprintf (fifo,CLIENT"%d", new->pid);

                        fd_cl = open (fifo, O_WRONLY);
                        if (fd_cl == -1) {
                            perror("Server: Error opening client FIFO");
                            free (new);
                            _exit (127);
                        }
                        
                        //Send the number of task to client
                        if (write(fd_cl, &ntask, sizeof(int)) == -1){
                            perror("Server: Error writing to client FIFO");
                            _exit (127);

                        }
                        
                        free (new);
                        close (fd_cl);
                        _exit (0);
                    }
                }
            }
        }
        else {
            //Wait till a task finishes
            wait(NULL);
                parallel_tasks--;
        }
		free (new);
	}

	for (int j=0; j<parallel_tasks; j++) {
		wait(NULL);
	}

	close(fd_sv);
	unlink(SERVER);
	

	return 0;
}