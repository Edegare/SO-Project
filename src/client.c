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

	// check if arguments valid in lenght and number
	if (argc < 2 || argc > 5) {
		printf("Missing argument or too many arguments.\n");
		_exit(1);
	}

	int total_length = 0;
    for (int i = 1; i < argc; i++) {
        total_length += strlen(argv[i]);
    }

    if (total_length > 300) {
        printf("Total length of arguments exceeds 300 bytes.\n");
        _exit(1);
    }

	// Malloc of the message to send 
	Msg mens = malloc(sizeof(struct msg));
	if (mens==NULL) {
		perror ("malloc client");
        _exit(1);
	}
    
    mens->pid = getpid(); 

	// Check arguments passed to client
    if (strcmp(argv[1], "status") == 0) {
        mens->time=0;
        mens->option=0;
		mens->status=0;
		strcpy(mens->message, "");
    } else if (strcmp(argv[1], "execute") == 0) {

        if (strcmp(argv[3], "-u")==0) mens->option=1;
		else if (strcmp(argv[3], "-p")==0) mens->option=2;
		else {printf("Unknow option: %s\n", argv[3]); free(mens);_exit(1);}
		mens->time=atoi(argv[2]);
		mens->status=0;
        snprintf(mens->message, sizeof(mens->message), "%s", argv[4]);
    
	} else if (strcmp(argv[1], "end") == 0) {
		mens->time=0;
		mens->option=3;
		mens->status=0;
		strcpy(mens->message, "");
	} else {
        printf("Unknown command: %s\n", argv[1]);
		free(mens);
        _exit(1);
    }

	// Create client fifo
	char fifo[30]; 
	sprintf (fifo,CLIENT"%d", mens->pid);
	if (mkfifo(fifo, 0666) == -1) {
        perror("Client: Error creating client FIFO");
        free (mens);
        _exit(1);
    }
    
	// Open server fifo to send commands
	int fd_sv = open(SERVER, O_WRONLY);  
	if (fd_sv==-1) {perror("Client: Error opening server FIFO");free (mens);_exit(1);}
	write(fd_sv, mens, sizeof(struct msg));
    close(fd_sv);


	if (mens->option==1 || mens->option==2) {
		int id;
		int fd_cl = open(fifo, O_RDONLY);  
		if (fd_cl==-1) {perror("Client: Error opening client FIFO");free (mens);_exit(1);}

		if (read(fd_cl, &id, sizeof(int))>0){
			printf("TASK %d RECEIVED\n", id);
		}
		close(fd_cl);
	}
	else if (mens->option==0) {

		// Receive Output folder from server
		char output_folder[32];
		int fd_cl = open(fifo, O_RDONLY);  
		if (fd_cl==-1) {perror("Client: Error opening client FIFO");free (mens);unlink(fifo);_exit(1);}

		if (read(fd_cl, output_folder, sizeof(output_folder))<=0){
			perror("Client: Error opening client FIFO");

		}
		close(fd_cl);

		char file_path_exec[64];
		snprintf(file_path_exec, sizeof(file_path_exec), "%s/%s", output_folder, EXECUTING);

		printf("Executing:\n");
		// Open a file for writing executing tasks
		int file_exec= open(file_path_exec, O_RDONLY);
		if (file_exec == -1) {
			//perror("Client: Error opening file");
		}
		else {
			Msg message = malloc(sizeof(struct msg)); 

			if (message == NULL) {
				perror("Client: Error malloc message");
			}
			else {
				ssize_t bytes_read;
				while ((bytes_read = read(file_exec, message, sizeof(struct msg))) > 0) {
					printf("%d %s\n",message->pid, message->message);
					memset(message, 0, sizeof(struct msg));
				}
			}
			close(file_exec);
			free(message);
		}

		char file_path_sched[64];
		snprintf(file_path_sched, sizeof(file_path_sched), "%s/%s", output_folder, QUEUE);

		printf("\nScheduled:\n");
		// Open a file for writing scheduled tasks
		int file_sched= open(file_path_sched, O_RDONLY);
		if (file_sched == -1) {
			//perror("Client: Error opening file");
		}
		else {
			Msg message = malloc(sizeof(struct msg)); 

			if (message == NULL) {
				perror("Client: Error malloc message");
			}
			else {
				ssize_t bytes_read;
				while ((bytes_read = read(file_sched, message, sizeof(struct msg))) > 0) {
					printf("%d %s\n",message->pid, message->message);
					memset(message, 0, sizeof(struct msg));
				}
			}
			close(file_sched);
			free(message);
		}

		char file_path_comp[64];
		snprintf(file_path_comp, sizeof(file_path_comp), "%s/%s", output_folder, COMPLETE);
		
		printf("\nCompleted:\n");
		// Open a file for writing completed tasks
		int file_comp= open(file_path_comp, O_RDONLY);
		if (file_comp == -1) {
			//perror("Client: Error opening file");
		}
		else {
			Msg message = malloc(sizeof(struct msg)); 

			if (message == NULL) {
				perror("Client: Error malloc message");
			}
			else {
				ssize_t bytes_read;
				while ((bytes_read = read(file_comp, message, sizeof(struct msg))) > 0) {
					printf("%d %s %d ms\n",message->pid, message->message, message->time);
					memset(message, 0, sizeof(struct msg));
				}
			}
			close(file_comp);
			free(message);
		}
		
	}
	else if (mens->option==3) {
		printf("Server going down...\n");
	}

	
	free (mens);
	unlink(fifo);
	return 0;
}