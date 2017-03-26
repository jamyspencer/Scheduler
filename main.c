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
#include <time.h>
#include <unistd.h>
#include "forkerlib.h"
#include "obj.h"



void PrintList(void);
void AbortProc();
void AlarmHandler();

static struct list* queue_0;
static struct list* queue_1;
static struct list* queue_2;

static struct list* executing_process = NULL;
static pcb_t* control_blocks;
static struct timespec* my_clock;
static int shmid[2];
static int messenger;
static char* file_name;

int main ( int argc, char *argv[] ){

	file_name = "test.out";
	int c, i;

	int num_users = 5;
	int max_run_time = 20;
	int child_count = 0;
	int total_spawned = 0;
	int pcb_loc;
	pcb_t this_pcb;
	struct timespec overhead;

	pid_t returning_child = 0;

	signal(2, AbortProc);
	signal(SIGALRM, AlarmHandler);
	queue_0 = NULL;

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

	//clear log file_name
	FILE* file_write = fopen(file_name, "w+");
	fclose(file_write);

	//init random numbers
	srand(time(0));


	//Create a timespec var to store when the next process should be spawned
	struct timespec when_next_fork;
	zeroTimeSpec(&when_next_fork);
	

	//Create an int bit vector to track pcbs that are active 
	int pcb_states[1];
	pcb_states[0] = 0;	


	//Initiallize shared memory space
	shrMemMakeAttach(shmid, &control_blocks, &my_clock);
	zeroTimeSpec(my_clock);
	for (i = 0; i < MAX_USERS; i++){
		(control_blocks + i)->pid = -1;
	}
		
	//set up lock queue with a message to allow the os in the first time
	messenger = lockMsgMakeAttach();
	msg_t *os_msg;
	os_msg = malloc (sizeof(msg_t) + 4);
	(*os_msg).mtype = 1;
	strncpy((*os_msg).mtext, "main", 4);
	if ((msgsnd(messenger, os_msg, sizeof(msg_t) + 4, 0)) == -1){
		perror("msgsnd, initial message");
	}

	//set up msg_t for users
	msg_t* user_msg;
	user_msg = malloc(sizeof(msg_t) + 1);
	strncpy((*user_msg).mtext, "y", 2);
	msg_t* unlock;
	unlock = malloc(sizeof(msg_t) + 11);


	do{//looping

		if((msgrcv(messenger, unlock, sizeof(msg_t) + MAX_MSG_LEN, 1, 0)) ==-1){
			perror("msgrcv");
		}
		//Create new user if it is time.
		if (cmp_timespecs(*my_clock, when_next_fork) >= 0 && total_spawned < 100 && my_clock->tv_sec < 2){
			pcb_loc = GetEmptyPCB(pcb_states, control_blocks);
			if (pcb_loc != -1){
				queue_0 = MakeChild(queue_0, control_blocks + pcb_loc, pcb_loc);
				if (queue_0 == NULL){
					perror("MakeChild failed");
					AbortProc();			
				}
				total_spawned++;
				child_count++;
				SaveLog(file_name, (control_blocks + pcb_loc)->pid, *my_clock, 0, "create");
				addLongToTimespec(rand() % MAX_SPAWN_DELAY + 1, &when_next_fork);
			}
		}


		//Scheduler, first check if there is user process that has just ceded control of crit section
		if (executing_process != NULL){
			pcb_loc = executing_process->item.pcb_location;
			this_pcb = *(control_blocks + pcb_loc);
			SaveLog(file_name, this_pcb.pid, this_pcb.this_burst, 0, "return");
			if (isTimeZero(this_pcb.tot_time_left)){
				CLEAR_BIT(pcb_states, pcb_loc);
				zeroTimeSpec(&this_pcb.tot_time_left);
				zeroTimeSpec(&this_pcb.tot_time_running);
				this_pcb.pid = 0;
				this_pcb.priority = 0;
			}
			else{
				//if process was interrupted and current priority queue is less than 2 then increment priority
				if (this_pcb.is_interrupt){
					SaveLog(file_name, this_pcb.pid, this_pcb.this_burst, 0, "not_done");
					this_pcb.priority = 0;
					queue_0 = PushProcess(queue_0, executing_process);
					SaveLog(file_name, executing_process->item.process_id, *my_clock, 0, "enqueue");
				}
				else if(this_pcb.priority == 0){
					(this_pcb.priority)++;
					queue_1 = PushProcess(queue_1, executing_process);
					SaveLog(file_name, executing_process->item.process_id, *my_clock, 1, "enqueue");
				}
				else if(this_pcb.priority == 1){
					(this_pcb.priority)++;
					queue_2 = PushProcess(queue_2, executing_process);
					SaveLog(file_name, executing_process->item.process_id, *my_clock, 2, "enqueue");
				}
				else {
					queue_2 = PushProcess(queue_2, executing_process);
					SaveLog(file_name, executing_process->item.process_id, *my_clock, 2, "enqueue");
				}

			}
		}
		//Schedule a process
		if (queue_0){
			executing_process = PopProcess(&queue_0);
			SaveLog(file_name, executing_process->item.process_id, *my_clock, 0, "dispatch");
		}
		else if (queue_1){
			executing_process = PopProcess(&queue_1);
			SaveLog(file_name, executing_process->item.process_id, *my_clock, 1, "dispatch");
		}
		else if (queue_2){
			executing_process = PopProcess(&queue_2);
			SaveLog(file_name, executing_process->item.process_id, *my_clock, 2, "dispatch");
		}
		else{
			executing_process = NULL;
			SaveLog(file_name, 0, *my_clock, 1, "no_process");
		}

		//advance system clock for scheduler overhead
		zeroTimeSpec(&overhead);
		addLongToTimespec((rand() % MAX_OVERHEAD  + 1), &overhead);
		plusEqualsTimeSpecs(my_clock, &overhead);

		SaveLog(file_name, 0, overhead, 0, "d_final");

		//send message to user process, or back to OS if no user process
		if (executing_process != NULL){
			user_msg->mtype = executing_process->item.process_id;
			if((msgsnd(messenger, user_msg, sizeof(msg_t) + MAX_MSG_LEN, 0)) == -1){
				perror("msgsnd -> user: in scheduler");
			}
		}
		else{
			if((msgsnd(messenger, os_msg, sizeof(msg_t) + MAX_MSG_LEN, 0)) == -1){
				perror("msgsnd -> os: in scheduler");
			}
		}

		if ((returning_child = waitpid(-1, NULL, WNOHANG)) != 0){
			if (returning_child != -1){
				queue_0 = destroyNode(queue_0, returning_child, file_name);
//				printf("Child %d returned/removed\n", returning_child);
				child_count--;
			}
		}
/*	if (child_count < 2){
		PrintList();
	}
     struct msqid_ds info;   
    if (msgctl(messenger, IPC_STAT, &info))
            perror("msgctl IPC_STAT error ");

    printf("Current # of messages on queue\t %d\n", info.msg_qnum);
	printf("pid of last sender %d vs pid of oss %d\n", info.msg_lspid, getpid());
*/

//	printf("Total Users: %d \t Active users: %d\n", total_spawned, child_count);	
	}while(child_count > 0 || (my_clock->tv_sec < 2 && total_spawned < 100));

	free(os_msg);
	free(unlock);
	free(user_msg);
	msgctl(messenger, IPC_RMID, NULL);
	shmdt(my_clock);
	shmdt(control_blocks);
	shmctl(shmid[0], IPC_RMID, NULL);
	shmctl(shmid[1], IPC_RMID, NULL);
	return 0;
}

void AlarmHandler(){
	perror("Time ran out");
	AbortProc();
}

void AbortProc(){

	while (queue_0 != NULL){	
		kill(queue_0->item.process_id, SIGKILL);
		destroyNode(queue_0, (queue_0->item).process_id, file_name);	
	}
	while (queue_1 != NULL){	
		kill(queue_1->item.process_id, SIGKILL);
		destroyNode(queue_1, (queue_1->item).process_id, file_name);	
	}
	while (queue_2 != NULL){	
		kill(queue_2->item.process_id, SIGKILL);
		destroyNode(queue_2, (queue_2->item).process_id, file_name);	
	}
	if (executing_process != NULL){	
		kill(executing_process->item.process_id, SIGKILL);
		destroyNode(executing_process, (executing_process->item).process_id, file_name);	
	}
	kill(0, 2);
	msgctl(messenger, IPC_RMID, NULL);
	shmdt(control_blocks);
	shmdt(my_clock);
	shmctl(shmid[1], IPC_RMID, NULL);
	shmctl(shmid[0], IPC_RMID, NULL);
	msgctl(messenger, IPC_RMID, NULL);
	exit(1);
}

void PrintList(void){
	struct list* this = queue_0;
	
	while (this){
		printf("pid: %d\n", this->item.process_id);
		this = this->next;
	}
	printf("*****END*******\n");
}
