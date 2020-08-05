#ifndef __IFMO_DISTRIBUTED_CLASS_LIST__H
#define __IFMO_DISTRIBUTED_CLASS_LIST__H

#include "ipc.h"
#include "process.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

struct ProcQueue
{
	pid_t		id;
	timestamp_t	time;
};

struct Node
{
	struct ProcQueue data;
	struct Node * next;
	struct Node * prev;
};

int AddInQueue(struct ProcQueue val);
void DeleteFromQueue(struct ProcQueue val);
//void FindFirstInQueue(struct ProcQueue * min);
int GetQueueLen();
void ShowAllNodes();
struct Node * GetNode(struct ProcQueue val);
struct ProcQueue FindFirstInQueue();
#endif
