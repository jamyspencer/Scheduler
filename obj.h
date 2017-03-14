/* Written by Jamy Spencer 23 Feb 2017 */
#ifndef OBJ_H
#define OBJ_H

#ifndef MAX_USERS
#define MAX_USERS 18
#endif
#ifndef BILLION
#define BILLION 1000000000
#endif
#ifndef MAX_MSG_LEN
#define MAX_MSG_LEN 12
#endif
#ifndef CHANCE_OF_INTERRUPT
#define CHANCE_OF_INTERRUPT 35
#endif
#ifndef QUANTUM
#define QUANTUM 100000
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
	struct timespec runtime;
	struct timespec last_burst;
	struct timespec cpu_time;
	struct timespec system_time;
	int priority;
}pcb_t;

typedef struct queue_msg{
	long int mtype;
	pid_t pid;
	char mtext[1];
} msg_t;

struct timespec* shrMemMakeAttach(int* shmid);
int lockMsgMakeAttach(void);
struct timespec addTimeSpecs(struct timespec t1, struct timespec t2);
int t1_grtr_eq_than_t2(struct timespec t1, struct timespec t2);

#endif
