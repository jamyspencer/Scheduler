/* Written by Jamy Spencer 23 Feb 2017 */
#ifndef OBJ_H
#define OBJ_H

#ifndef MAX_USERS
#define MAX_USERS 18
#endif
#ifndef MAX_SPAWN_DELAY
#define MAX_SPAWN_DELAY 2000000
#endif
#ifndef BILLION
#define BILLION 1000000000
#endif
#ifndef MAX_MSG_LEN
#define MAX_MSG_LEN 12
#endif
#ifndef CHANCE_OF_INTERRUPT
#define CHANCE_OF_INTERRUPT 50
#endif
#ifndef MAX_TOTAL_RUNTIME
#define MAX_TOTAL_RUNTIME 100000000
#endif
#ifndef QUANTUM
#define QUANTUM 4000000
#endif
#ifndef MAX_OVERHEAD
#define MAX_OVERHEAD 1000
#endif
#ifndef GO
#define GO 1
#endif
#ifndef STOP
#define STOP 0
#endif
#ifndef SET_BIT
#define SET_BIT(var,pos)     ( var[(pos/32)] |= (1 << (pos%32)) )
#endif
#ifndef CLEAR_BIT 
#define CLEAR_BIT(var,pos)   ( var[(pos/32)] &= ~(1 << (pos%32)) )            
#endif
#ifndef CHECK_BIT
#define CHECK_BIT(var,pos) ( var[(pos/32)] >> (pos%32) & 1)
#endif

typedef struct pcb{
	pid_t pid;
	struct timespec this_burst;
	struct timespec tot_time_running;
	struct timespec tot_time_left;
	int priority;
	int is_interrupt;
	int pcb_loc;
	int quantum;
	int percent_interrupt;
}pcb_t;

typedef struct queue_msg{
	long int mtype;
	char mtext[1];
} msg_t;

int isTimeZero(struct timespec t1);
void shrMemMakeAttach(int* shmid, pcb_t** cntl_blocks, struct timespec** clock);
int lockMsgMakeAttach(void);
void zeroTimeSpec(struct timespec* t1);
struct timespec divTimeSpecByInt(struct timespec dividend, int divisor);
void plusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2);
void minusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2);
void addLongToTimespec(long l, struct timespec* t1);
int cmp_timespecs(struct timespec t1, struct timespec t2);
long pwr(long n, long p);
void log_mem_loc(pcb_t* addr, char* exec);

#endif
