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
    char output_folder[16];
    snprintf(output_folder, sizeof(output_folder), "%s", argv[1]);
    //Number of max parallel taks
    int parallel_tasks_max= atoi(argv[2]);

    if (parallel_tasks_max<1 || parallel_tasks_max>10) {
        printf("Number of parallel tasks not recomended. 1 to 10 parallel tasks possible.\n");
		_exit(1);
    }



    // flag 
    int flag_policy=atoi(argv[3]);

    GQueue *queue;
    GArray *array_execution = g_array_new(FALSE, FALSE, sizeof(Msg));
    if (array_execution == NULL) {
        perror("Server: Failed to create GArray");
        _exit(1);
    }

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
                            sleep(2);
                            gettimeofday(&end, NULL);
                            long seconds = end.tv_sec - start.tv_sec;
                            long useconds = end.tv_usec - start.tv_usec;
                            int task_time = ((seconds) * 1000 + useconds/1000.0);

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
                    }
                }
            }
            else {
                if (new->option==0) {
                    //If status option

                    // Executing tasks file 
                    char file_path_exec[64];
                    snprintf(file_path_exec, sizeof(file_path_exec), "%s/%s", output_folder, EXECUTING);

                    // Open a file for writing executing tasks
                    int file= open(file_path_exec, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (file == -1) {
                        perror("Server: Error opening file");
                    }
                    else {
                        for(int i=0; i<array_execution->len; i++) {
                            // Write to the file
                            Msg msg = g_array_index(array_execution, Msg, i);
                            if (msg == NULL) {
                                perror("Server: g_array_index error\n");
                            }
                            else {
                                ssize_t bytes_written = write(file, msg, sizeof(struct msg));
                                if (bytes_written == -1) {
                                    perror("Server: Error writing to file"); 
                                }
                            }
                        }
                        close(file);
                    }

                    // Queue tasks file 
                    char file_path_queue[64];
                    snprintf(file_path_queue, sizeof(file_path_queue), "%s/%s", output_folder, QUEUE);

                    // Open a file for writing queue tasks
                    int file_queue= open(file_path_queue, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (file_queue == -1) {
                        perror("Server: Error opening file");
                    }
                    else {
                        GQueue *tmp_queue = g_queue_copy(queue);
                        while (!g_queue_is_empty(tmp_queue)) {
                            Msg queue_task = g_queue_pop_head(tmp_queue);
                            if (queue_task == NULL) {
                                perror("Server: queue_task error\n");
                            }
                            else {
                                ssize_t bytes_written = write(file_queue, queue_task, sizeof(struct msg));
                                if (bytes_written == -1) {
                                    perror("Server: Error writing to file"); 
                                }
                            }
                            free(queue_task);
                        }
                        g_queue_free(tmp_queue);
                        close(file_queue);
                    }
                    
                }
                else {
                    // If task finished, put in file
                    if (new->status==2) {

                        char file_path[64];
                        snprintf(file_path, sizeof(file_path), "%s/%s", output_folder, COMPLETE);

                        // Open a file for writing
                        int file= open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
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

                                int s=0;

                                for (int i = array_execution->len - 1; s != 1 && i >= 0; i--) {
                                    Msg msg = g_array_index(array_execution, Msg, i);
                                    if (msg == NULL) {
                                        perror("Server: g_array_index error\n");
                                    }
                                    else {
                                        printf("%d\n", msg->pid);
                                        if (msg->pid == new_pid) {
                                            g_array_remove_index(array_execution, i);
                                            free(msg);
                                            s = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // Tasks from clients
                    else if (new->status==0) {
                        char fifo[30];
                        sprintf (fifo,CLIENT"%d", new->pid);

                        fd_cl = open (fifo, O_WRONLY);
                        if (fd_cl == -1) {
                            perror("Server: Error opening client FIFO");
                        }
                        else {
                            Msg task_queue = malloc(sizeof(struct msg));
                            if (task_queue == NULL) {
                                perror("Server: malloc task_queue");
                            } 
                            else {
                                // Copy the contents of new to task_queue
                                memcpy(task_queue, new, sizeof(struct msg));

                                // Put the task_queue in the queue
                                g_queue_push_tail(queue, task_queue);
                                
                                // Send the task ID to the client
                                int id = task_queue->pid;
                                if (write(fd_cl, &id, sizeof(int)) == -1) {
                                    perror("Server: Error writing to client FIFO");
                                }
                            }
                            // Close the client FIFO
                            close(fd_cl);
                        }
                    }

                    // While queue not empty and at least one child is waiting for a task
                    while (array_execution->len < parallel_tasks_max && !(g_queue_is_empty(queue))) {
                        Msg m = g_queue_pop_tail(queue);
                        m->status=1;
                        // Send to pipe of childs
                        if (write(pipeQueue[1], m,sizeof(struct msg)) == -1){
                            perror("Server: Error writing to pipeQueue");
                        }
                        else {
                            // Send to execution array
                            Msg new_m = malloc(sizeof(struct msg));
                            if (new_m == NULL) {
                                perror("Server: malloc new_m");
                            } else {

                                memcpy(new_m, m, sizeof(struct msg));

                                g_array_append_val(array_execution, new_m);
                            }
                        }
                        free(m);
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