/* Written by Jamy Spencer 23 Feb 2017 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "obj.h"



int main ( int argc, char *argv[] ){

	int doing_it = GO;
	int array_loc;
	long this_quantum;
	pcb_t* my_pcb = NULL;
	pid_t my_pid = getpid();
	struct timespec run_time;

	//initiallize locking message
	msg_t *msg_to_oss;
	msg_to_oss = malloc(sizeof(msg_t) + 6);
	(*msg_to_oss).mtype = 1;
	snprintf((*msg_to_oss).mtext, 8, "%06d", my_pid);
	int lock_que = lockMsgMakeAttach();


	//initiallize unlocking message
	msg_t *unlock;
	unlock = malloc(sizeof(msg_t) + 11);

	//get shared memory
	int shmid[2];
	struct timespec* my_clock;
	pcb_t* control_blocks;
	shrMemMakeAttach(shmid, &control_blocks, &my_clock);

	//initiallize rand generator
	srand(my_pid);

	//Calculate total run-time
	run_time.tv_sec = 0;
	run_time.tv_nsec = 0;
	addLongToTimespec(rand() % MAX_TOTAL_RUNTIME + 1, &run_time);

	

	while (doing_it){
	//Critical Section--------------------------------------------------------
		if((msgrcv(lock_que, unlock, sizeof(msg_t) + MAX_MSG_LEN, my_pid, 0)) == -1){
			printf("msgrcv, slave, pid:%d\n", my_pid);
		}

		//get array location of this processes pcb_location if not already done
		if(my_pcb == NULL){
			for (array_loc = 0; array_loc < MAX_USERS; array_loc++){
				if ((control_blocks + array_loc)->pid == my_pid){
					my_pcb = (control_blocks + array_loc);
					zeroTimeSpec(&(my_pcb->tot_time_left));
					assign_t1_t2(&(my_pcb->tot_time_left), &run_time);
					break;
				}
			}
//log_mem_loc( (control_blocks + array_loc), "user1");				
		}


		//determine if the whole quantum will be used and assign a run-time accordingly
		zeroTimeSpec(&(my_pcb->this_burst));
		if (((rand() % 100) + 1) < (my_pcb->percent_interrupt)){
			this_quantum = rand() % ((my_pcb->quantum) * pwr(2, (my_pcb->priority)));
			my_pcb->is_interrupt = 1;
		}
		else{
			this_quantum = (my_pcb->quantum) * pwr(2, (my_pcb->priority));
			my_pcb->is_interrupt = 0;
		}
		addLongToTimespec(this_quantum, &(my_pcb->this_burst));


		//Check that burst-time doesn't exceed the time needed

		if (cmp_timespecs(my_pcb->this_burst, my_pcb->tot_time_left) > 0){
			assign_t1_t2(&(my_pcb->this_burst), &(my_pcb->tot_time_left));
			my_pcb->is_interrupt = 0;
		}

		plusEqualsTimeSpecs(&(my_pcb->tot_time_running), &(my_pcb->this_burst));
		plusEqualsTimeSpecs(my_clock, &(my_pcb->this_burst));
		minusEqualsTimeSpecs(&(my_pcb->tot_time_left), &(my_pcb->this_burst));
//log_mem_loc(my_pcb, "user2");
		//user is done, close down everything
		if (isTimeZero(my_pcb->tot_time_left)){
			doing_it = STOP;
//printf("this burst %02lu:%09lu\n",my_pcb->this_burst.tv_sec, my_pcb->this_burst.tv_nsec );
			shmdt(control_blocks);
			shmdt(my_clock);

		}
		if ((msgsnd(lock_que, msg_to_oss, sizeof(msg_t) + MAX_MSG_LEN, 0)) == -1){
			perror("msgsnd");
		}
	//End Critical Section---------------------------------------------------			
	}

	free (msg_to_oss);
	free (unlock);

	return 0;
}


