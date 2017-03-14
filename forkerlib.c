/* Written by Jamy Spencer 23 Feb 2017 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h> 
#include <time.h>
#include <unistd.h>
#include "forkerlib.h"
#include "obj.h"


pcb_t* GetEmptyPCB(int* pcb_states, pcb_t* head_ptr){
	int i = 0;
	while (CHECK_BIT(pcb_states, i) && i < MAX_USERS){
		i++;
	}
	return (head_ptr + i);
}

struct list* PopProcess(struct list *queue_head){
	struct list* temp = queue_head;
	queue_head = temp->next;
	queue_head->prev = NULL;
	temp->next = NULL;
	temp->prev = NULL;
	return temp;
}

void PushProcess(struct list* queue_head, struct list* process){
	struct list* tail = returnTail(queue_head);
	tail->next = process;
	process->prev = tail;
}

struct list *MakeChild(int* head_ptr, pcb_t* my_pcb, pid_t pid){

	pid = fork();
	if (pid < 0){
		perror("Fork failed");
		return NULL;
	}
	else if (pid > 0){			
		head_ptr = addNode(head_ptr, pid);
		my_pcb->pid = pid;

	}
	else if (pid == 0){
		execl("./user", "user", (char*) NULL);
	}	
	return head_ptr;
}

struct list *addNode(struct list *head_ptr, pid_t pid){
	struct list *temp = malloc (sizeof(struct list));

	temp->item.process_id = pid;
	temp->next = NULL;

	if (head_ptr == NULL){
		temp->prev = NULL;
		return temp;
	}
	else{
	struct list *tail = returnTail(head_ptr);
	tail->next = temp;
	temp->prev = tail;
	}
	return head_ptr;
}

struct list *returnTail(struct list *head_ptr){
	struct list *temp = head_ptr;
	while(temp != NULL && temp->next){
		temp = temp->next;	
	}
	return temp;
}

struct list* findNodeByPid(struct list *head_ptr, pid_t pid){
	struct list *temp = head_ptr;
	while(temp != NULL){
		if (temp->item.process_id == pid){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

int SaveLog(char* log_file_name, pid_t pid, struct timespec clock, int queue, char* log_type) {
	FILE* file_write = fopen(log_file_name, "a");

	if (strcmp(log_type, "create") == 0){
		fprintf(log_file_name, "OSS: Generating process with PID %d and putting it in queue 1 at time %02lu:%09lu\n", pid, clock.tv_sec, clock.tv_nsec); 
	} 
	else if (strcmp(log_type, "dispatch") == 0){
		fprintf(log_file_name, "OSS: Dispatching process with PID %d from queue %d at time %02lu:%09lu\n", pid, queue, clock.tv_sec, clock.tv_nsec);
	}
	else if (strcmp(log_type, "d_final") == 0){
		fprintf(log_file_name, "OSS: total time this dispatch was %02lu:%09lu\n", clock.tv_sec, clock.tv_nsec);
	}
	else if (strcmp(log_type, "enqueue") == 0){
		fprintf(log_file_name, "OSS: Putting process with PID %d into queue %d\n", pid, queue);
	}
	else if (strcmp(log_type, "terminate") == 0){
		fprintf(log_file_name, "OSS: Receiving that process with PID %d ran for %02lu:%09lu nanoseconds\n", pid, clock.tv_sec, clock.tv_nsec);
	}
	else if (strcmp(log_type, "not_done") == 0){
		fprintf(log_file_name, "OSS: not using its entire time quantum\n");
	}
	fclose(file_write);
	return 0;
}

//returns the head_ptr address of the list that now has the node containing the pid passed removed
struct list* destroyNode(struct list *head_ptr, pid_t pid, char* file_name){
	struct list *temp = findNodeByPid(head_ptr, pid);
	struct list *new_head;


	if (temp == NULL){
		printf("Couldn\'t find %d ", pid);
		return head_ptr;
	}
	else if (temp == head_ptr){//delete head
		if(temp){
			new_head = temp->next;//set new head to next
			if (new_head){//set the new heads prev to NULL
				new_head->prev = NULL;
			}
			free(temp);
			temp = NULL;		
		}
		else{perror("attempted to destroy null head");}
		return new_head;
	}
	else if (temp->next == NULL){//delete tail
		(temp->prev)->next = NULL;
		free(temp);
		return head_ptr;
	}
	else{//delete a mid

		(temp->prev)->next = temp->next;
		(temp->next)->prev = temp->prev;
		free(temp);
		return head_ptr;
	}
	return NULL;
}

void KillSlaves(struct list *head_ptr, char* file_name){
	while (head_ptr != NULL){	
		kill(head_ptr->item.process_id, SIGKILL);
		destroyNode(head_ptr, (head_ptr->item).process_id, file_name);	
	}
}


void clock_tick(struct timespec *clock, int increment){

	while (increment){
		if (clock->tv_nsec < (BILLION - increment)){
			(clock->tv_nsec) += increment;
			increment = 0;
		}
		else{	
			(clock->tv_sec)++;
			clock->tv_nsec = ((clock->tv_nsec)	 + increment - BILLION);
		}
	}
//	fprintf(stderr, "SYSTEM CLOCK: %lu %lu\n", clock->tv_sec, clock->tv_nsec);
}
