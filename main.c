#include "common.h"
#include "process.h"
#include "ipc.h"

pid_t pid = 1;


int main(int argc, char *argv[])
{
	//printf("I am %d parent process\n", (int)getpid());
	chProcAmount = 0;
	currentID = 0;
	childPID[PARENT_ID] = getpid();

	localTime = 0;

	if((chProcAmount = CheckOptionAndGetValue(argc, argv)) <= 0)
	{
		printf("Must specify -p option...");
		exit(1);
	}

	//Создание дескрипторов файлов для логирования pipe и events		
	FILE *fileEv, *filePipe;

	//Открытие файла для добавления логов каналов
	filePipe = fopen(pipes_log, "w");

	//Создание pipe-ов каналов, pipe[Из][Куда] 
	//например pipe[1][2] передает от доч.проц. 1 в доч.проц. 2
	CreatePipes(chProcAmount, filePipe);	

	fclose(filePipe);
	filePipe = fopen(pipes_log, "a");

	//Открытие файла для добавления логов событий
	fileEv = fopen(events_log, "a");
	
	//Создание дочерних процессов
	for(int i = 1; i <= chProcAmount; i++)
	{
		pid = fork();	//Возвращает PID дочернего процесса, если он все еще в родительском процессе		
			if(pid == -1)	//Не удалось создать дочерний процесс
			{	
				printf("Error on creating child %d. Exiting...\n", i);
				exit(0);
			}
			//Выполняется в дочернем процессе
			if(pid == 0)	//Сейчас в дочернем процессе
			{
				currentID = i;	//currentID хранит локальный id процесса
				break;			//выйти из цикла, ибо в дочернем незачем создавать процессы
			}
		//printf("Child %d was created %d\n", i, pid);
		childPID[i] = pid;
	}

	//2 дочерних -> 6 каналов, 
	for(int i = 0; i <= chProcAmount; i++)
	{
		for(int j = 0; j <= chProcAmount; j++)
		{
			if(i != currentID && i != j)
			{
				//Например, в дочернем процессе currentID = 2 
				//Закрыть pipe[0][1], pipe[1][0]  на запись
				close(pipes[i][j].field[WRITE]);
				WritePipeLog (filePipe, i, j, "WRITE", (int)getpid());
			}
			if(j != currentID && i != j)
			{
				//Например, в дочернем процессе currentID = 2 
				//Закрыть pipe[1][0], pipe[0][1]  на чтение
				close(pipes[i][j].field[READ]);
				WritePipeLog (filePipe, i, j, "READ", (int)getpid());
			}
		}
	}
	
	///В дочернем процессе
	if(pid == 0)	//Один из дочерних процессов
	{			
		Message msg;
		struct ProcQueue currentState = {currentID, get_lamport_time ()};

		int resultStarted = 0;
		int from = 0;
		doneCounter = 0;
		//printf("I am %d child with PID %d and balance %d\n", currentID, (int)getpid(), balance.s_balance);

		localTime++;
		Message msgStart = { {MESSAGE_MAGIC, 0, STARTED, get_lamport_time ()}, };
		sprintf(msgStart.s_payload, log_started_fmt, (int)get_lamport_time (), currentID, (int)getpid(), (int)getppid(), 0);
		WriteEventLog(log_started_fmt, fileEv, get_lamport_time (), currentID, (int)getpid(), (int)getppid(), 0);
		//printf(log_started_fmt, get_lamport_time (), currentID, (int)getpid(), (int)getppid(), balance.s_balance);
		printf("%s", msgStart.s_payload);
		msgStart.s_header.s_payload_len = strlen(msgStart.s_payload);
		
		//Рассылка всем процессам сообщений STARTED из текущего дочернего процесса
		if(send_multicast(&currentID, &msgStart) == UNSUCCESS)
		{
			exit(UNSUCCESS);
		}
		//Получение STARTED сообщений от дочерних процессов
		for(int i = 1; i <= chProcAmount; i++)
		{
			if(i != currentID)
				if(receive(&currentID, i, &msg) == STARTED)
				{
					resultStarted++;		//увеличение счетчика сообщений STARTED от доч. проц.
					localTime = MAX(localTime, msg.s_header.s_local_time);
					localTime++;
					//printf("%d: process %d received STARTED from process %d\n", get_lamport_time (), currentID, i);
				}
		}

		for(int i = 0; i < sizeof(doneChilds); i++)
			doneChilds[i] = 0;
		
		if(resultStarted == chProcAmount - 1)
		{
			printf(log_received_all_started_fmt, get_lamport_time (), currentID);
			WriteEventLog(log_received_all_started_fmt, fileEv, get_lamport_time (), currentID);
			char str[64] = {0};

			localTime++;		//"!!!" - тут
			currentState.time = get_lamport_time ();
			currentState.id = currentID;
			//printf("process %d state is [%d, %d], flag = %d\n", currentID, (int)currentState.id, (int)currentState.time, flagCS);

			if(flagCS > 0)
			{
				for(int i = 1; i <= currentID * 5; i ++)
				{
					if(request_cs(&currentState) == SUCCESS)
					{
						//log_loop_operation_fmt = "process %1d is doing %d iteration out of %d\n";
						sprintf(str, log_loop_operation_fmt, currentID, i, currentID * 5);
						print(str);
						release_cs(&currentState);
						//if(release_cs(&currentState) == 0)
							//printf("%d SUCCESSFULLY RELEASED\n", currentID);
						currentState.time = get_lamport_time ();
					}
				}
			}
			else
			{
				for(int i = 1; i <= currentID * 5; i ++)
				{
					//log_loop_operation_fmt = "process %1d is doing %d iteration out of %d\n";
					sprintf(str, log_loop_operation_fmt, currentID, i, currentID * 5);
					//printf(log_loop_operation_fmt, currentID, i, currentID * 5);
					print(str);
				}
			}
		}
							
		Message doneMsg = {{MESSAGE_MAGIC, 0, DONE, get_lamport_time ()}, };
		sprintf(doneMsg.s_payload, log_done_fmt, (int)get_lamport_time (), currentID, 0);
		WriteEventLog(log_done_fmt, fileEv, get_lamport_time (), currentID, 0);
		//printf(log_done_fmt, get_lamport_time (), currentID, balance.s_balance);
		printf("%s", doneMsg.s_payload);
		doneMsg.s_header.s_payload_len = strlen(doneMsg.s_payload);
		
		send_multicast (&currentID, &doneMsg);
		
		//Получение DONE сообщений от дочерних процессов
		if(doneCounter != chProcAmount - 1)
		{	//если процесс завершился раньше крайнего, то ему нужно дождаться DONE всех
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
					if(msg.s_header.s_type == DONE)
					{
						doneCounter++;		//увеличение счетчика сообщений DONE от доч. проц.
						if(doneCounter == chProcAmount - 1)
						{
							WriteEventLog(log_received_all_done_fmt, fileEv, get_lamport_time (), currentID);
							printf(log_received_all_done_fmt, get_lamport_time (), currentID);
							exit(SUCCESS);	//завершение текущего доч. проц. с кодом 0 (SUCCESS)
						}
					}
				}
			}//while(1)
		}
		else
		{
			WriteEventLog(log_received_all_done_fmt, fileEv, get_lamport_time (), currentID);
			printf(log_received_all_done_fmt, get_lamport_time (), currentID);
			exit(SUCCESS);	//завершение текущего доч. проц. с кодом 0 (SUCCESS)
		}
	}
	else
	if(PARENT_ID == currentID)	//Родительский процесс, currentID = 0
	{
		//printf("Waiting child process ending...\n");

		int countStarted = 0;
		int countDone = 0;
		Message msg;
		
		//Получение STARTED сообщений от дочерних процессов
		for(int i = 1; i <= chProcAmount; i++)
		{
			if(receive(&currentID, i, &msg) == STARTED)
			{
				countStarted++;		//увеличение счетчика сообщений STARTED от доч. проц.
				localTime = MAX(localTime, msg.s_header.s_local_time);
				localTime++;
				//printf("%d: process 0 received STARTED from process %d\n", get_lamport_time (), i);
			}
		}
		
		//Проверка получения STARTED от всех дочерних процессов
		if(countStarted == chProcAmount)
		{
			WriteEventLog(log_received_all_started_fmt, fileEv, get_lamport_time(), currentID);
			printf(log_received_all_started_fmt, get_lamport_time(), currentID);
			
			for(int i = 1; i <= chProcAmount; i++)
			{
				if(receive(&currentID, i, &msg) == DONE)
				{
					countDone++;		//увеличение счетчика сообщений  от доч. проц.
					localTime = MAX(localTime, msg.s_header.s_local_time);
					localTime++;
					//printf("%d: process 0 received %s from process %d\n", get_lamport_time (), messageType[msg.s_header.s_type], i);
				}
			}
			if(countDone == chProcAmount)
			{
				WriteEventLog(log_received_all_done_fmt, fileEv, get_lamport_time(), currentID);
				printf(log_received_all_done_fmt, get_lamport_time (), currentID);

				fclose (filePipe);
				fclose(fileEv);

				while(wait(NULL))
				{
					if(errno == ECHILD)
						break;
				}
				return SUCCESS;
			}
		}
		return UNSUCCESS;
	}
}


timestamp_t get_lamport_time()
{
	return localTime;
}
