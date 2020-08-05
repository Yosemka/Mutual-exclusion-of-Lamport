#include "process.h"

int request_cs(const void * self)
{
	Message msg;
	int countReply = chProcAmount - 1;
	int from = 0;
	//увеличение времени перед записью текущего состояния процесса помечено "!!!"
	//Отправить всем запрос запрос на вход в КС, REQUEST
	Message msgReq = {{MESSAGE_MAGIC, 0, CS_REQUEST, get_lamport_time ()}, };

	for(int i = 0; i < sizeof(doneChilds); i++)
		if(doneChilds[i] == i * 5 && i != 0)
			countReply--;

	//send(&currentID, 0, &msgReq);
	for(int i = 1; i <= chProcAmount; i++)
	{
		if(i != currentID && doneChilds[i] != i * 5)
		{
			send(&currentID, i, &msgReq);
		}
	}
	//send_multicast (&currentID, &msgReq);
	struct ProcQueue currentState = *(struct ProcQueue *)self;
	//printf("PROCESS %d requests for CS with state is [%d, %d] countReply = %d\n", currentID, (int)currentState.id, (int)currentState.time, countReply);
	AddInQueue(currentState);
	//ShowAllNodes();
	if(countReply == 0)
		return SUCCESS;

	while(1)
	{
		from = receive_any (&currentID, &msg);
		if(from == UNSUCCESS)
		{
			exit(UNSUCCESS);
		}
		else
		{
			localTime = MAX(localTime, msg.s_header.s_local_time);
			localTime++;
			currentState.time = msg.s_header.s_local_time;
			currentState.id = from;
			switch(msg.s_header.s_type)
			{
				case CS_REQUEST:
				{
					//printf("%d: process %d received CS_REQUEST from process %d local_time = %d countReply = %d\n", get_lamport_time (), currentID, from, msg.s_header.s_local_time, countReply);
					
					AddInQueue(currentState);
					//ShowAllNodes ();
					//printf("First in Queue %d: [%d, %d]\n", currentID, first.id, first.time);

					first = FindFirstInQueue();
					localTime++;
					Message reply = {{MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time ()}, };
					send (&currentID, from, &reply);
					break;
				}
				case CS_REPLY:
				{
					//printf("%d: process %d received CS_REPLY from process %d local_time = %d\n", get_lamport_time (), currentID, from, msg.s_header.s_local_time);
					
					countReply--;			//увеличение счетчика сообщений CS_REPLY от доч. проц.
					break;
				}
				case CS_RELEASE:
				{
					doneChilds[from]++;
					//printf("%d: process %d received CS_RELEASE from process %d local_time = %d doneChilds[%d] = %d countReply = %d\n", 
					//       get_lamport_time (), currentID, from, msg.s_header.s_local_time, from, doneChilds[from], countReply);
					
					DeleteFromQueue(currentState);
					first = FindFirstInQueue();
					//ShowAllNodes();
					//printf("First in Queue %d: [%d, %d]\n", currentID, first.id, first.time);
					if(doneChilds[from] == from * 5 && countReply != 0)
					   countReply--;
					
					break;
				}
				case DONE:
				{
					doneCounter++;		//счетчик сообщений DONE здесь нужен, тк 
					//			 процесс, тоот, кто еще не завершиполучит это сообщение в этой функциился, 
				}
				default: break;
			}
			if(countReply == 0)
			{
				if(first.id == currentID)	
				{	//если первый в очереди тот, у которого id совпадает со своим, то
					return SUCCESS;	//дать ему пропуск
				}
				
			}
		}
	}
	
	return UNSUCCESS;
}

int release_cs(const void * self)
{
	//Отправить всем сообщение о выходе из КС, CS_RELEASE
	localTime++;
	struct ProcQueue currentState = *(struct ProcQueue *)self;
	DeleteFromQueue(currentState);
	//ShowAllNodes ();
	first = FindFirstInQueue ();
	Message msgRep = {{MESSAGE_MAGIC, 0, CS_RELEASE, currentState.time}, };

	//send(&currentID, 0, &msgRep);
	for(int i = 1; i <= chProcAmount; i++)
	{
		if(i != currentID)
		{
			if(doneChilds[i] != i * 5)
			{
				send(&currentID, i, &msgRep);
			}
		}
	}
	
	//send_multicast (&currentID, &msgRep);
	return 0;
}

void CreatePipes(int procAmount, FILE * file)
{
	int countPipes = 0;		//счетчик активных/созданных/используемых каналов
	int flag;
	for(int i = 0; i <= procAmount; i++)	//Всего каналов надо ([количество доч. проц.] + 1)*2
	{
		for(int j = 0; j <= procAmount; j++)
		{
			
			if(i != j)		//pipe[i][i] внутри одного процесса не нужны
			{
				//if(pipe2(pipes[i][j].field, O_NONBLOCK) == -1)	//Ошибка при создании канала
				if(pipe(pipes[i][j].field) == -1)	//Ошибка при создании канала
				{
					printf("Error on creating pipe %d -> %d. Exiting...\n", i, j);
					exit(0);
				}
				else
				{
					flag = fcntl(pipes[i][j].field[READ], F_GETFL, 0);
					fcntl(pipes[i][j].field[READ], F_SETFL, flag | O_NONBLOCK);

					flag = fcntl(pipes[i][j].field[WRITE], F_GETFL, 0);
					fcntl(pipes[i][j].field[WRITE], F_SETFL, flag | O_NONBLOCK);

					if(fprintf(file, "Pipe from %d to %d created, R: %d  W: %d\n",
						   i, j, pipes[i][j].field[READ], pipes[i][j].field[WRITE]) == 0)
					{
						printf("Error writing on \"%s\" pipe[%d][%d]", pipes_log, i, j);
					}

					//printf("Pipe from ch[%d] to ch[%d] created, R: %d  W: %d\n",
					//	   i, j, pipes[i][j].field[READ], pipes[i][j].field[WRITE]);
					
					countPipes++;	//увеличение счетчика при успешном создании канала
				}
			}
		}
	}
	printf("%d Pipes created\n", countPipes);
}

/*
 *  text - константные значения сообщений из файла "pa1.h"
 *  file - дескриптор открытого лог-файла (сделать проверку)
 */
void WriteEventLog(const char *text, FILE *file, ...)
{	
	va_list vars;			//Хранит список аргументов после аргумента file
	va_start(vars, file);	//из библиотеки <stdarg.h>
	//Применение описано https://metanit.com/cpp/c/5.13.php

	if(vfprintf(file, text, vars) == 0)	//Неуспешная запись в лог-файл
	{
		printf ("Error writing on \"event.log\" ");
		printf(text, vars);
	}
	va_end(vars);
}

void WritePipeLog(FILE *file, int from, int to, char* type, int curPID)
{ 
	if(fprintf(file, "Pipe from %d to %d closed to %s in process %d\n", from, to, type, curPID) == 0)
	{
		printf("Error writing on \"pipes.log\" closing pipe from %d to %d to %s in process %d\n", from, to, type, curPID);
	}
}

/*
 * Проверка атрибута после симвода 'p'
 */
int CheckOptionAndGetValue(int argc, char *argv[])
{
	int option;
	int childAmount = 0;

	int opt_index = 0;
	static struct option opt[] =
	{
		{"proc", required_argument, 0, 'p'},
		{"mutexl", no_argument, 0, 'm'},
		{NULL, 0, 0, 0}
	};
	
	flagCS = 0;
	//while((option = getopt(argc, argv, "p:m")) != UNSUCCESS)	//"p:" - двоеточие говорит, что p обязателен
	while((option = getopt_long(argc, argv, "p:m", opt, &opt_index)) > 0)
	{
		switch(option)	//getopt возвращает символ аргумента, а optarg хранит значение аргумента
		{				//то есть optarg количество доч. процессов
			case('p'):
			{
				//printf("p Argumnet %s\n%d\n", optarg, argc);
				if((childAmount = atoi(optarg)) == 0)
				{
					printf("Incorrect 'child process amount' value");
					return UNSUCCESS;
				}
				else if(childAmount > MAX_PROCESS_ID)	//Если число выше MAX_PROCESS_ID (15)...
				{
					printf("'child process amount' couldnt be more than %d", MAX_PROCESS_ID); 
					childAmount = MAX_PROCESS_ID;		//... то установить количество доч. проц. равным MAX_PROCESS_ID
				}
				/*if(argc - 3 == 1)
				{
					if(strncmp(argv[3], "--mutexl", sizeof("--mutexl")) == 0)	//argv[0]=main.exe, argv[1]=-p, argv[2]=childAmount, argv[3]='--mutexl' и т.д.
						flagCS = 1;
				}*/
				break;
			}				
			case('m'): printf("he %s\n", optarg); flagCS = 1; break;
			case('?'): printf("Option -p needs an argument"); return UNSUCCESS;
			default: printf("No such option"); return UNSUCCESS;
		}
	}
	return childAmount;
}

char IsOnlyDigits(char* str)
{
	for(int i = 0; i < strlen(str); i++)
	{
		if(str[i] == '0' ||
		   str[i] == '1' ||
		   str[i] == '2' ||
		   str[i] == '3' ||
		   str[i] == '4' ||
		   str[i] == '5' ||
		   str[i] == '6' ||
		   str[i] == '7' ||
		   str[i] == '8' ||
		   str[i] == '9')
			return SUCCESS;
		else
			return UNSUCCESS;
	}
	return UNSUCCESS;
}
