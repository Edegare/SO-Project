#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <glib.h>

#include "defines.h"
#include "aux.h"

/* executar o servidor:
$ ./orchestrator output_folder parallel-tasks sched-policy
Argumentos:
1. output-folder: pasta onde são guardados os ficheiros com o output de tarefas executadas.
2. parallel-tasks: número de tarefas que podem ser executadas em paralelo.
3. sched-policy: identificador da política de escalonamento, caso o servidor suporte várias políticas. */

int main (int argc, char * argv[]){
	struct timeval start, end;

    if (argc!=4) {
        printf("Missing argument or too many arguments.\n");
		_exit(1);
	}

    // Arguments received
    char output_folder[8];
    strcpy(output_folder, argv[1]);
    //Number of max parallel taks
    int parallel_tasks_max= atoi(argv[2]);

    if (parallel_tasks_max<1 || parallel_tasks_max>10) {
        printf("Number of parallel tasks not recomended. 1 to 10 parallel tasks possible.\n");
		_exit(1);
    }



    // flag 
    int flag_policy=atoi(argv[3]);

    GQueue *queue;
    GArray *array_execution = g_array_new(FALSE, FALSE, sizeof(int));

    if (flag_policy==0) { 
        // first in first out - Queue
        queue = g_queue_new();

    }
   /*  else if (flag_policy==1) {
        // less time

    } */
    else {
        printf("Not an existent sched policy.\n");
        _exit(1);
    }

	int stop=0;
    int pipeQueue[2];

    // Pipe creation - comunication with tasks between dad process and child ones 
    if (pipe(pipeQueue)==-1) {
        perror ("Server: pipeQueue creation");
        return -1;
    }

    // Server FIFO creation
	if (mkfifo(SERVER, 0666) == -1) {perror("Server: Error creating server FIFO");_exit(1);}
	

    int fd_sv_rd, fd_sv_wr, fd_cl;


	fd_sv_rd = open (SERVER, O_RDONLY);
	if (fd_sv_rd == -1) {
		perror("Server: Error opening server FIFO");
		_exit(1);
	}
    fd_sv_wr = open (SERVER, O_WRONLY);
	if (fd_sv_wr == -1) {
		perror("Server: Error opening server FIFO");
		_exit(1);
	}

    // == CHILD DOES TASKS ==
    for (int i=0; i<parallel_tasks_max; i++) {
        int bytes_read_child;

            int fork_ret= fork();

            if (fork_ret == -1) {
                perror ("fork");
                i--;
            }

            else if (fork_ret==0) {
                
                close(pipeQueue[1]);

                while (!stop){
                    Msg task= malloc(sizeof(struct msg));
                    if (task==NULL) {
                        perror ("Server: malloc server message task");
                        _exit (127);
                    }
                    if ((bytes_read_child=read(pipeQueue[0],task,sizeof(struct msg))) > 0){
                        if (task->option==3) stop=1;
                        else {
                            //parse new->message to commands[][] array TODO before the rest 
                            gettimeofday(&start, NULL);
                            // DO TASK AND CREATES A FILE WITH OUTPUT......

                            gettimeofday(&end, NULL);
                            long seconds = end.tv_sec - start.tv_sec;
                            long useconds = end.tv_usec - start.tv_usec;
                            int task_time = ((seconds) * 1000 + useconds/1000.0);
                            printf("TASK %d DONE\n", task->pid);
                            //Change status task to finished and time to real time
                            task->time=task_time;
                            task->status=2;
                            if (write(fd_sv_wr, task, sizeof(struct msg)) == -1){
                                perror("Server: Error in Child writing to server FIFO");
                            }
                        }
                    }
                    free(task);
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
        else if ((bytes_read=read(fd_sv_rd,new,sizeof(struct msg))) > 0){

            // If client says to end, stop cycle
            if (new->option==3) {
                stop=1;
                for (int i=0;i<parallel_tasks_max; i++){
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
                        // If task finished, put in file
                        if (new->status==2) {
                            char file_path[32];
                            snprintf(file_path, sizeof(file_path), "%s/%s", output_folder, COMPLETE);

                            // Open a file for writing
                            int file= open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (file == -1) {
                                perror("Server: Error opening file");
                            }
                            else {
                                // Write to the file
                                ssize_t bytes_written = write(file, new, sizeof(struct msg));
                                if (bytes_written == -1) {
                                    perror("Server: Error writing to file");
                                    close(file);
                                }
                                else {
                                    // Close the file
                                    close(file);

                                    //Remove task from execution array because it is finished now
                                    int new_pid = new->pid;
                                    if (find_msg_remove(array_execution, new_pid)==FALSE) {
                                        perror("Server: Error finding message to remove from execution array");
                                    }
                                }
                            }
                        }
                        else if (new->status==0) {
                            fd_cl = open (fifo, O_WRONLY);
                            if (fd_cl == -1) {
                                perror("Server: Error opening client FIFO");
                            }
                            else {
                                // If task is coming from client put in queue
                                g_queue_push_tail(queue, new);
                                int id=new->pid;
                                //Send the number of task to client
                                if (write(fd_cl, &id, sizeof(int)) == -1){
                                    perror("Server: Error writing to client FIFO");
                                }
                                close (fd_cl);
                            }
                        }

                        guint array_size = g_array_get_element_size(array_execution);
                        // While queue not empty and at least one child is waiting for a task
                        while (array_size < parallel_tasks_max && g_queue_is_empty(queue)) {
                            Msg m = g_queue_pop_tail(queue);
                            m->status=1;
                            if (write(pipeQueue[1], m,sizeof(struct msg)) == -1){
                                perror("Server: Error writing to pipeQueue");
                                int error=-1;
                                if (write(fd_cl,&error, sizeof(int)) == -1){
                                    perror("Server: Error writing to client FIFO");
                                }
                            }
                            else {
                                g_array_append_vals(array_execution, m,1);
                            }
                        }
                    }
                }
                
            }
        }
		free (new);
	}

    close(pipeQueue[0]);
    close(pipeQueue[1]);

    // Wait for childs
	for (int j=0; j<parallel_tasks_max; j++) {
		wait(NULL);
	}

    g_array_free(array_execution, TRUE);
    if (flag_policy==0) { 
        // first in first out - Queue
        g_queue_free(queue);
        
    }
    /* else if (flag_policy==1) {
        // less time

    } */

	close(fd_sv_rd);
    close(fd_sv_wr);
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
/* while(parallel_tasks<parallel_tasks_max && tasks_queue>0) {

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