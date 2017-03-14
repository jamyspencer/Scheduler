/* Written by Jamy Spencer 23 Feb 2017 */
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h>
#include "obj.h"
#include <unistd.h>



pcb_t* shrMemMakeAttach(int* shmid){

	pcb_t *shrd_data;
	key_t key;

	/* make the key: */
    if ((key = ftok("main.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((*shmid = shmget(key, sizeof(struct timespec) + sizeof(pcb_t) * MAX_USERS, IPC_CREAT | 0644)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    shrd_data = shmat(*shmid, (void*) NULL, 0);
    if (shrd_data == (void *)(-1)) {
        perror("shmat");
        exit(1);
    }
	return shrd_data;
}

int lockMsgMakeAttach(void){

	int msgque;
	key_t key;

	/* make the key: */
    if ((key = ftok("main.c", 'X')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((msgque = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }	
	return msgque;
}

struct timespec addTimeSpecs(struct timespec t1, struct timespec t2){
	struct timespec temp;

	temp.tv_sec = t1.tv_sec + t2.tv_sec;
	temp.tv_nsec = t1.tv_nsec + t2.tv_nsec;

	if(temp.tv_sec >= BILLION){
		temp.tv_nsec -= BILLION;
		(temp.tv_sec)++;
	}
	return temp;
}

int t1_grtr_eq_than_t2(struct timespec t1, struct timespec t2){
	if (t1.tv_sec >= t2.tv_sec && t1.tv_sec >= t2.tv_sec) {return GO;}
	return STOP;
}
