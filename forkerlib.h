/* Written by Jamy Spencer 23 Feb 2017 */
#ifndef FORKERLIB_H
#define FORKERLIB_H

#include "obj.h"

struct info{
	pid_t process_id;
}; 

struct list{
	struct info item;
	struct list* next;
	struct list* prev;
};

pcb_t* GetEmptyPCB(int* pcb_states, pcb_t* head_ptr);
struct list* PopProcess(struct list *queue_head);
void PushProcess(struct list* queue_head, struct list* process);
struct list *MakeChild(int* head_ptr, pcb_t* my_pcb, pid_t pid);
void KillSlaves(struct list *hd_ptr, char* file_name);
int SaveLog(char* log_file_name, pid_t pid, struct timespec clock, int queue, char* log_type);
void clock_tick(struct timespec *clock, int increment);
struct list *returnTail(struct list *head_ptr);
struct list *addNode(struct list *head_ptr, pid_t pid);
struct list* destroyNode(struct list *head_ptr, pid_t pid, char* file_name);

struct list* findNodeByPid(struct list *head_ptr, pid_t pid);

#endif
