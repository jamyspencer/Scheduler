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
	struct timespec now;
	struct timespec start;
	struct timespec run_time;
	struct timespec x_time;
	

	int doing_it = GO;
	int lock_len = 1;

	//initiallize locking message
	msg_t *my_lock;
	my_lock = malloc(sizeof(msg_t) + 6);
	(*my_lock).mtype = 3;
	snprintf((*my_lock).mtext, 6, "%04d", getpid());
	int lock_que = lockMsgMakeAttach();


	//initiallize unlocking message
	msg_t *unlock;
	unlock = malloc(sizeof(msg_t) + 11);

	//get shared memory
	int shr_mem_id;
	pcb_t* control_blocks = shrMemMakeAttach(&shr_mem_id);
	struct timespec* my_clock = control_blocks[MAX_USERS];	


	//set run time
	srand(time(NULL));
	run_time.tv_sec = 0;
	run_time.tv_nsec = (rand() % 100000) + 1;

	x_time = addTimeSpecs(start, run_time);

	while (doing_it){
	//Critical Section--------------------------------------------------------
		if((msgrcv(lock_que, my_lock, sizeof(msg_t) + 11, 3, 0)) == -1){
			perror("msgrcv, slave");
		}

		now.tv_sec = sys_clock->tv_sec;
		now.tv_nsec = sys_clock->tv_nsec;
		doing_it = t1_grtr_than_t2(x_time, now);

		if (doing_it == STOP){

			//initiallize exit message
			msg_t* x_msg;
			x_msg = malloc(sizeof(msg_t) + 11);
			x_msg->mtype = 1;
			x_msg->pid = getpid();
			snprintf(x_msg->mtext, MAX_MSG_LEN,"%02lu%09lu", now.tv_sec, now.tv_nsec);

			//initiallize receiver message
			msg_t *shut_down;
			shut_down = malloc (sizeof(msg_t) + lock_len);

			shmdt(sys_clock);

			if((msgsnd(lock_que, x_msg, sizeof(msg_t), 0)) == -1){
				perror("msgsnd: x_msg");
			}
			if((msgrcv(lock_que, shut_down, sizeof(msg_t) + 1, 9, 0)) == -1){
				perror("msgrcv, returning");
			}
			free (x_msg);
			free (shut_down);
		}
//		fprintf(stderr, "Exiting: %d\n", getpid());

		if ((msgsnd(lock_que, my_lock, sizeof(msg_t) + lock_len, 0)) == -1){
			perror("msgsnd");
		}
		
	//End Critical Section---------------------------------------------------			
	}

	free (my_lock);
	free (unlock);

	return 0;
}



	





