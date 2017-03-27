/* Written by Jamy Spencer 23 Feb 2017 */
#ifndef FORKERLIB_H
#define FORKERLIB_H

#include "obj.h"

struct stats{
	struct timespec tot_user_wait;
	struct timespec tot_user_runtime;
	struct timespec tot_user_lifetime;
	struct timespec cpu_idle_time;
	struct timespec temp;	
	int child_count;	
	int total_spawned;
};

struct info{
	pid_t process_id;
	int pcb_location;
	struct timespec t_zero;
	struct timespec t_final;
}; 

struct list{
	struct info item;
	struct list* next;
	struct list* prev;
};

int GetEmptyPCB(int* pcb_states, pcb_t* head_ptr);
struct list* PopProcess(struct list **queue_head);
struct list* PushProcess(struct list* queue_head, struct list* process);
struct list *MakeChild(struct list* head_ptr, pcb_t* my_pcb, int pcb_loc, struct timespec clock);
void KillSlaves(struct list *hd_ptr, char* file_name);
int SaveLog(char* log_file_name, pid_t pid, struct timespec clock, int queue, char* log_type);
void LogStats(char* file_name, struct stats inf);
struct list *returnTail(struct list *head_ptr);
struct list *addNode(struct list *head_ptr, pid_t pid, int pcb_loc,struct timespec clock);
struct list* destroyNode(struct list *head_ptr, pid_t pid, char* file_name);

struct list* findNodeByPid(struct list *head_ptr, pid_t pid);

#endif
