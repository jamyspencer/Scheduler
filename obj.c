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



void shrMemMakeAttach(int* shmid, pcb_t** cntl_blocks, struct timespec** clock){
	/* make the key: */
	int key[2];

    if ((key[0] = ftok("main.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((key[1] = ftok("slave.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid[0] = shmget(key[0], sizeof(pcb_t) * MAX_USERS, IPC_CREAT | 0644)) == -1) {
        perror("shmget");
        exit(1);
    }
    if ((shmid[1] = shmget(key[1], sizeof(struct timespec), IPC_CREAT | 0644)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    *cntl_blocks = shmat(shmid[0], (void*) NULL, 0);
    if (*cntl_blocks == (void *)(-1)) {
        perror("shmat");
        exit(1);
    }
    *clock = shmat(shmid[1], (void*) NULL, 0);
    if (*clock == (void *)(-1)) {
        perror("shmat");
        exit(1);
    }
	return;
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

int isTimeZero(struct timespec t1){
	if (t1.tv_sec == 0 && t1.tv_nsec == 0) return 1;
	return 0;
}

void zeroTimeSpec(struct timespec* t1){
	t1->tv_sec = 0;
	t1->tv_nsec = 0;
	return;
}

struct timespec divTimeSpecByInt(struct timespec dividend, int divisor){
	struct timespec result;

	result.tv_sec = (dividend.tv_sec) / divisor; 
	result.tv_nsec = (unsigned long) (((dividend.tv_sec) % divisor) * BILLION /divisor); 
	dividend.tv_nsec = dividend.tv_nsec / divisor;
	(result.tv_nsec) += (dividend.tv_nsec);
	if(result.tv_nsec >= BILLION){
		result.tv_nsec -= BILLION;
		(result.tv_sec)++;
	}
	return result;	
}

void plusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2){

	t1->tv_sec = t1->tv_sec + t2->tv_sec;
	t1->tv_nsec = t1->tv_nsec + t2->tv_nsec;

	if(t1->tv_nsec >= BILLION){
		t1->tv_nsec -= BILLION;
		(t1->tv_sec)++;
	}
	return;
}

void minusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2){

	t1->tv_sec = t1->tv_sec - t2->tv_sec;
	t1->tv_nsec = t1->tv_nsec - t2->tv_nsec;

	if(t1->tv_nsec < 0){
		t1->tv_nsec += BILLION;
		(t1->tv_sec)--;
	}
	return;
}

void addLongToTimespec(long l, struct timespec* t1){
	t1->tv_nsec = t1->tv_nsec + l;

	if(t1->tv_nsec >= BILLION){
		t1->tv_nsec -= BILLION;
		(t1->tv_sec)++;
	}
	return;
}

int cmp_timespecs(struct timespec t1, struct timespec t2){
	if (t1.tv_sec > t2.tv_sec) return 1;
	else if (t1.tv_sec < t2.tv_sec) return -1;
	else if (t1.tv_nsec > t2.tv_nsec) return 1;
	else if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
} 

long pwr(long n, long p){
	if (p == 0){return 1;}
	return pwr(n, p-1) * n;
}

void log_mem_loc(pcb_t* addr, char* exec){
	FILE* file_write = fopen("memlog.out", "a");
	fprintf(file_write, "PID:%6d PCB: %03d", addr->pid, addr->pcb_loc); 
	fprintf(file_write, " Memory  Address: %09x, Burst: %09lu, Executable: %s\n", &(*addr), addr->this_burst.tv_nsec, exec); 
	fclose(file_write);
	return;
}

