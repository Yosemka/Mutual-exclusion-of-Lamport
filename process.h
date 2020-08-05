#ifndef __IFMO_DISTRIBUTED_CLASS_PROCESS__H
#define __IFMO_DISTRIBUTED_CLASS_PROCESS__H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include "pa2345.h"
#include "banking.h"
#include "list.h"

#define READ 0
#define WRITE 1
#define SUCCESS 0
#define UNSUCCESS -1
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct 
{
	int 		field[2];
}Pipes;

int 		chProcAmount;
pid_t 		childPID[MAX_PROCESS_ID];
local_id 	currentID;
int 		pidBalance[MAX_PROCESS_ID];
char		flagCS;		//флаг 0 - программа запускается без использования критической секции, 1 - с КС
struct ProcQueue minInQueue;
Pipes 		pipes[MAX_PROCESS_ID][MAX_PROCESS_ID];
//ProcQueue	queue[MAX_PROCESS_ID];
timestamp_t localTime;
static char* messageType[] = {"STARTED", "DONE", "ACK", "STOP", "TRANSFER", "BALANCE_HISTORY", "CS_REQUEST", "CS_REPLY", "CS_RELEASE"};
struct ProcQueue first;

/*счетчик принтов каждого процесса. Нужен для того, чтобы не отправлять завершенному процессу сообщений и не зависать.
Сообщения DONE поступали слишком поздно*/
int			doneChilds[MAX_PROCESS_ID + 1];	

int			doneCounter;		//здесб объявлена, чтобы была видна и в process.c, и в main

//прототипы
int CheckOptionAndGetValue(int, char**);
void CreatePipes(int, FILE*);
void CreateChilds(int);
void WriteEventLog(const char *, FILE *, ...);
void WritePipeLog(FILE *, int, int, char*, int);
char IsOnlyDigits(char* str);

#endif 
