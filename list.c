#include "list.h"

struct Node *head = NULL;
int nodesAmount = 0;


int AddInQueue(struct ProcQueue val) 
{
	//struct ProcQueue value = *(struct ProcQueue *)val;
	struct Node *tmp = (struct Node*) malloc(sizeof(struct Node));
	tmp -> data = val;

	if(head == NULL)
	{
		head = tmp;
		head->next = tmp;
		head->prev = tmp;
	}
	else
	{
		tmp->next = head;
		tmp->prev = head->prev;
		head->prev->next = tmp;
		head->prev = tmp;
	}
	nodesAmount++;
	//printf("Child %d added [id, time]: [%d, %d]\n", currentID, val.id, val.time);
	return nodesAmount;
}

void DeleteFromQueue(struct ProcQueue val) {

	struct Node *delete = GetNode(val);
	//printf("Node To Delete [%d, %d], amount %d\n", delete->data.id, delete->data.time, nodesAmount);
	if(nodesAmount <= 0)
	{
		return;
		//return nodesAmount;
	}
	else if(nodesAmount == 1)
	{
		free(head);
		head = NULL;
		nodesAmount = 0;
		return;
	}
	else
	{
		if(delete == head)
		{
			head = head->next;
		}
		
		delete->prev->next = delete->next;
		delete->next->prev = delete->prev;
		free(delete);
		nodesAmount--;
		return;
	}
}

void ShowAllNodes()
{
	struct Node *current = head;
	printf ("Process %d has in queue: ", currentID);
	if(nodesAmount > 0)
	{
		do
		{
			printf ("[%d, %d]  ", current -> data.id, current -> data.time);
			current = current -> next;
		}while(current != head);
		printf("\n");
	}
	else
		printf("nothing...\n");
}

struct Node * GetNode(struct ProcQueue val)
{
	struct Node *current = head;
	if(nodesAmount <= 0)
	{
		return NULL;
	}
	else
	{
		do
		{
			if(current->data.id == val.id && current->data.time == val.time)
			{
				return current;
			}
			current = current->next;
		}while(current != head);
	}
	return NULL;
}

struct ProcQueue FindFirstInQueue()
{
	struct Node *current = head;
	struct ProcQueue min = {0, 0};

	if(nodesAmount > 0)
	{
		min = current -> data;
		//printf("Searching starts from [%d, %d]\n", min.id, min.time);
		//current = current->next;
		do
		{
			if(current -> data.time < min.time)
			{
				min = current -> data;
			}
			else 
			if(current -> data.time == min.time)
			{
				if(current -> data.id < min.id)
					min = current -> data;
			}
			current = current->next;
		}while(current != head);
	}
	return min;
}

int GetQueueLen()
{
	return nodesAmount;
}
