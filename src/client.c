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
		strcpy(mens->message, "");
    } else if (strcmp(argv[1], "execute") == 0) {

        if (strcmp(argv[3], "-u")==0) mens->option=1;
		else if (strcmp(argv[3], "-p")==0) mens->option=2;
		else {printf("Unknow option: %s\n", argv[3]); free(mens);_exit(1);}
		mens->time=atoi(argv[2]);
        strncpy(mens->message, argv[4], sizeof(mens->message));
    
	} else if (strcmp(argv[1], "end") == 0) {
		mens->time=0;
		mens->option=3;
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
		int ntask;
		int fd_cl = open(fifo, O_RDONLY);  
		if (fd_cl==-1) {perror("Client: Error opening client FIFO");free (mens);_exit(1);}

		if (read(fd_cl, &ntask, sizeof(int))>0){
			printf("TASK %d RECEIVED\n", ntask);
		}
		close(fd_cl);
	}
	else if (mens->option==0) {
		//TO DO
		
	}
	else if (mens->option==3) {
		printf("Server going down...\n");
	}

	
	free (mens);
	unlink(fifo);
	return 0;
}