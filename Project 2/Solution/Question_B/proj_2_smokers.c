#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>

#include <sys/types.h>													 		/* libraries for shared memory functions*/                                
#include <sys/shm.h>
#include <sys/ipc.h>

#include <fcntl.h>
#include <semaphore.h>															/* library for POSIX semaphores */

#define  NUMBER_OF_SMOKERS 3													/* 3 child processes as smokers */ 

int main() 
{
	pid_t smokers[NUMBER_OF_SMOKERS];											/* smokers' pid table */

	int i;
						
	int child_status;

	key_t keyTable;																/* key for ftok() */

	int shmidTable;																/* shared memory id variables for shmget() */

	int *table;																	/* table is an array where table[0]:tobacco, table[1]:paper, table[2]:matches */

	/**** SYSTEM V IPC(InterProcess Communication) package functions ****/

	keyTable = ftok("table.txt", 's');											/* ftok - convert a pathname and a project identifier to a System V IPC key */

	if(keyTable == -1)
	{
		perror("Something went wrong with ftok(). \n");
		exit(1);
	}

	shmidTable = shmget(keyTable, NUMBER_OF_SMOKERS*sizeof(int), 0600 | IPC_CREAT);

	if(shmidTable == -1)
	{
		perror("Something went wrong with shmget(). \n");
		exit(1);
	}

	table = shmat(shmidTable, NULL, 0);

	if(table == (void *) -1)
	{
		perror("Something went wrong with shmat(). \n");
		exit(1);
	}

	/**** End of SYSTEM V IPC package functions ****/

	sem_t *sellerSem = sem_open ("sSem", O_CREAT, 0644, 2);
	sem_t *tobaccoSem = sem_open ("tSem", O_CREAT, 0644, 0);
	sem_t *paperSem = sem_open ("pSem", O_CREAT, 0644, 0);
	sem_t *matchSem = sem_open ("mSem", O_CREAT, 0644, 0);

	if (sellerSem == SEM_FAILED || tobaccoSem == SEM_FAILED || paperSem == SEM_FAILED || matchSem == SEM_FAILED)
	{
		printf("Semaphore initialization failed. Exiting... \n");
		exit(1);
	}

	for(i = 0; i < NUMBER_OF_SMOKERS; i++)										/* start with empty table */
	{
		table[i]=0;
	}

	printf("forking \n");


	for (i = 0; i < NUMBER_OF_SMOKERS; i++)
	{
		smokers[i] = fork();
		printf("hi\n");																/* fork() parent NUMBER_OF_SMOKERS times and log their pids in pid[] */

		if (smokers[i] == 0) 														/* if i-th process is first parent's child process (= fork() returned 0) */
		{	
			printf("hi2\n");	
			while(1)
			{
				printf("hi3\n");
				printf("%d\n",i );
				sleep(10);
				switch(i)
				{
					case 0:
						sem_wait(tobaccoSem);
						if(table[1]>0 && table [2]>0){
						table[1]--;
						table[2]--;}
						printf("Took paper and matches. \n");
						sem_post(sellerSem);
						break;
					case 1:
						sem_wait(paperSem);
						table[0]--;
						table[2]--;
						printf("Took tobacco and matches. \n");
						sem_post(sellerSem);
						break;
					case 2:
						sem_wait(matchSem);
						table[0]--;
						table[1]--;
						printf("Took tobacco and paper. \n");
						sem_post(sellerSem);
						break;
					default:
						printf("Something went wrong.");
				}
			}
		}
	}

	/* Seller's Code */
	printf("seller \n");

	while(1)
	{
		int choice = rand() % 3;	/* seller has 3 options to put to the table */

		printf("%d\n", choice );

		sem_wait(sellerSem);

		printf("passed semaphore\n");

		switch(choice)
		{
			case 0:
				table[0]++;
				table[1]++;
				printf("Placed tobacco and paper on the table. \n");
				sem_post(matchSem);
				break;
			case 1:
				table[0]++;
				table[2]++;
				printf("Placed tobacco and matches on the table. \n");
				sem_post(paperSem);
				break;
			case 2:
				table[1]++;
				table[2]++;
				printf("Placed paper and matches on the table. \n");
				sem_post(tobaccoSem);
				break;
			default:
				printf("Something went wrong.\n");
		}


	}

	// for (i = 0; i < NUMBER_OF_SMOKERS; i++) 
	// {
	// 	pid_t wpid = waitpid(pid[i], &child_status, 0);											/* execute waitpid() NUMBER_OF_SMOKERS
	// 	 times to wait for NUMBER_OF_SMOKERS
	// 	 children to exit */

	// 	if (WIFEXITED(child_status)) 															 WIFEXITED returns true if child exited normally 
	// 	{
	// 		printf("Child%d terminated with return value %d\n", wpid, WEXITSTATUS(child_status));		/* prints terminated child's id and its exit status */
	// 	}
	// 	else
	// 	{
	// 		printf("Child%d terminated abnormally\n", wpid);
	// 	}
	// }

	// print_shared_array(sharedArray, NUMBER_OF_SMOKERS);

	return (0);
}