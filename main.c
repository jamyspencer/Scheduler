/* Written by Jamy Spencer 23 Feb 2017 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/msg.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "forkerlib.h"
#include "obj.h"



void PrintList(void);
void AbortProc();
void AlarmHandler();

static struct list* queue_1;
static struct list* queue_2;
static struct list* queue_3;
static struct list* wait_queue;
static struct list* executing_process;
static pcb_t* control_blocks;
static int shmid;
static int lock_que_id;
static char* file_name;

int main ( int argc, char *argv[] ){

	file_name = "test.out";
	int c, i;

	int num_users = 5;
	int max_run_time = 20;
	int child_count = 0;
	int total_spawned = 0;

	pid_t returning_child;
	pid_t new_child_pid;

	signal(2, AbortProc);
	signal(SIGALRM, AlarmHandler);
	queue_1 = NULL;

	while ( (c = getopt(argc, argv, "hi:l:s:t:")) != -1) {
		switch(c){
		case 'h':
			printf("-h\tHelp Menu\n");
			printf("-l\tSet log file name(default is test.out)\n-s\tChanges the number of slave processes(default is 5)\n");
			printf("-t\tChanges the number of seconds to wait until the master terminates all slaves and itself(default is 20)\n");
			return 0;
			break;
		case 'l':
			file_name = optarg;
			break;
		case 's':
			num_users = atoi(optarg);
			if (num_users > MAX_USERS){
				printf("Error: -s exceeds MAX_USERS, set to %d\n", MAX_USERS);
				exit(1);
			}
			break;
		case 't':
			max_run_time = atoi(optarg);
			break;
		case '?':
			return 1;
			break;
		}
	}
	alarm(max_run_time);

	//init random numbers
	srand(time(0));

	//Create a timepspec var to store when the next process should be spawned
	struct timespec when_next_fork;
		when_next_fork.tv_nsec = 0;
		when_next_fork.tv_sec = 0;

	//Create an int bit vector to track pcbs that are active 
	int pcb_states[1];
	pcb_states[0] = 0;	

	//Create queues and a var for the process executing
	queue_1 = malloc(sizeof(struct list) * num_users);
	queue_2 = malloc(sizeof(struct list) * num_users);
	queue_3 = malloc(sizeof(struct list) * num_users);
	wait_queue = malloc(sizeof(struct list) * num_users);
	executing_process = NULL;

	//Initiallize shared memory space
	control_blocks = shrMemMakeAttach(&shmid);
	struct timespec* my_clock = (struct timepspec*) (control_blocks + MAX_USERS);

		
	//set up lock queue with a message to allow the os in the first time
	lock_que_id = lockMsgMakeAttach();
	msg_t *my_lock;
	my_lock = malloc (sizeof(msg_t) + 4);
	(*my_lock).mtype = 1;
	strncpy((*my_lock).mtext, "main", 4);
	if ((msgsnd(lock_que_id, my_lock, sizeof(msg_t) + 4, 0)) == -1){
		perror("msgsnd, initial message");
	}

	//set up msg_t to send acknowledgement to exiting users
	msg_t* xt_user;
	xt_user = malloc(sizeof(msg_t) + 1);
	(*xt_user).mtype = 9;
	strncpy((*xt_user).mtext, "y", 2);
	msg_t* unlock;
	unlock = malloc(sizeof(msg_t) + 11);


	do{//looping

		if((msgrcv(lock_que_id, unlock, sizeof(msg_t) + 11, 1, 0)) ==-1){
			perror("msgrcv");
		}

		if (t1_grtr_eq_than_t2(*my_clock, when_next_fork)){
			if (MakeChild(queue_1, GetEmptyPCB(pcb_states, control_blocks), new_child_pid) == NULL){
				perror("MakeChild failed");
				AbortProc();			
			}
			total_spawned++;
			child_count++;
			SaveLog(file_name, new_child_pid, *my_clock, 1, "create");
		}
//int SaveLog(char* log_file_name, pid_t pid, struct timespec clock, int queue, char* log_type

		//Scheduler
		

		if ((returning_child = waitpid(-1, NULL, WNOHANG)) != 0){
			if (returning_child != -1){
				queue_1 = destroyNode(queue_1, returning_child, file_name);
//				printf("Child %d returned/removed\n", returning_child);
				child_count--;
			}
		}
/*	if (child_count < 2){
		PrintList();
	}
     struct msqid_ds info;   
    if (msgctl(lock_que_id, IPC_STAT, &info))
            perror("msgctl IPC_STAT error ");

    printf("Current # of messages on queue\t %d\n", info.msg_qnum);
	printf("pid of last sender %d vs pid of oss %d\n", info.msg_lspid, getpid());
*/

//	printf("Total Users: %d \t Active users: %d\n", total_spawned, child_count);	
	}while(total_spawned < 100 && my_clock->tv_sec < 2);

	free(my_lock);
	free(unlock);
	free(xt_user);
	msgctl(lock_que_id, IPC_RMID, NULL);
	shmdt(my_clock);
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}

void AlarmHandler(){
	perror("Time ran out");
	AbortProc();
}

void AbortProc(){
	msgctl(lock_que_id, IPC_RMID, NULL);
	while (queue_1 != NULL){	
		kill(queue_1->item.process_id, SIGKILL);
		destroyNode(queue_1, (queue_1->item).process_id, file_name);	
	}
	while (queue_2 != NULL){	
		kill(queue_2->item.process_id, SIGKILL);
		destroyNode(queue_2, (queue_2->item).process_id, file_name);	
	}
	while (queue_3 != NULL){	
		kill(queue_3->item.process_id, SIGKILL);
		destroyNode(queue_3, (queue_3->item).process_id, file_name);	
	}
	while (wait_queue != NULL){	
		kill(wait_queue->item.process_id, SIGKILL);
		destroyNode(wait_queue, (wait_queue->item).process_id, file_name);	
	}
	if (executing_process != NULL){	
		kill(executing_process->item.process_id, SIGKILL);
		destroyNode(executing_process, (executing_process->item).process_id, file_name);	
	}
	msgctl(lock_que_id, IPC_RMID, NULL);
	shmdt(control_blocks);
	shmctl(shmid, IPC_RMID, NULL);
	kill(0, 2);
	msgctl(lock_que_id, IPC_RMID, NULL);
	exit(1);
}

void PrintList(void){
	struct list * this = queue_1;
	
	while (this){
		printf("pid: %d\n", this->item.process_id);
		this = this->next;
	}
	printf("*****END*******\n");
}
