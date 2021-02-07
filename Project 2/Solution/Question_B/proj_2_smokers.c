/**************************************************************************************************************************************************************************/
//	This C program solves the Smoker's Problem using semaphores.
//	At first the parent process (seller) creates 3 separate processes (smokers) using fork.
//	The parent and the child-processes all have acces to a shared array (table) with a size of 3(table[0]:tobacco, table[1]:paper, table[2]:matches)
//  The parent(seller) then randomly increments 2 out of 3 shared array positions (2 out of {tobacco,paper,matches}) and signals the child(smoker) that needs them.
//	The called child(smoker) then proceeds to decrement the 2 previously incremented array positions and signals back the father (seller) to start the procedure again.
//
//
//	The shared array was implemented using the System V IPC(InterProcess Communication) package, which is supported by Linux.
//	The semaphores where implemented using the POSIX Semaphores package which is also supported by Linux.
//	The code was written and tested on Ubuntu 20.04 with GNU C/C++ Compiler 9.3.0
//
/**************************************************************************************************************************************************************************/

#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>

#include <sys/types.h>													 				/* libraries for shared memory functions*/
#include <sys/shm.h>
#include <sys/ipc.h>

#include <fcntl.h>																		/* contains 0_CREAT value needed for sem_open */
#include <semaphore.h>																	/* library for POSIX semaphores */

#define  NUMBER_OF_SMOKERS 3															/* 3 child processes as smokers */

int main()
{
	pid_t smokers[NUMBER_OF_SMOKERS];													/* smokers' pid table */

	int child_status;

	key_t keyTable;																		/* key for ftok() */

	int shmidTable;																		/* shared memory id variables for shmget() */

	int *table;																			/* table is an array where table[0]:tobacco, table[1]:paper, table[2]:matches */


	/**** SYSTEM V IPC(InterProcess Communication) package functions ****/


	keyTable = ftok("table.txt", 's');													/* ftok() - convert a pathname and a project identifier to a System V IPC key */

	if(keyTable == -1)
	{
		perror("Something went wrong with ftok(). \n");
		exit(1);
	}

	shmidTable = shmget(keyTable, NUMBER_OF_SMOKERS*sizeof(int), 0600 | IPC_CREAT);		/* shmget() - allocates a System V shared memory segment (0600 gives rw permission to user) */

	if(shmidTable == -1)
	{
		perror("Something went wrong with shmget(). \n");
		exit(1);
	}

	table = shmat(shmidTable, NULL, 0);													/* shmat() - attaches the System V shared memory segment identified by shmid to the address space of the calling process */

	if(table == (void *) -1)
	{
		perror("Something went wrong with shmat(). \n");
		exit(1);
	}


	/**** End of SYSTEM V IPC package functions ****/


	sem_t *sellerSem = sem_open ("/ssem", O_CREAT, 0600, 1);							/* seller's semaphore is initialized with 1 */
	sem_t *tobaccoSem = sem_open ("/tsem", O_CREAT, 0600, 0);							/* smokers' semaphores are initialized with 0 */
	sem_t *paperSem = sem_open ("/psem", O_CREAT, 0600, 0);
	sem_t *matchSem = sem_open ("/msem", O_CREAT, 0600, 0);

	if (sellerSem == SEM_FAILED || tobaccoSem == SEM_FAILED || paperSem == SEM_FAILED || matchSem == SEM_FAILED)
	{
		printf("Semaphore initialization failed. Exiting... \n");
		exit(1);
	}

	for(int i = 0; i < NUMBER_OF_SMOKERS; i++)											/* start with an empty table */
	{
		table[i]=0;
	}

	for (i = 0; i < NUMBER_OF_SMOKERS; i++)
	{
		smokers[i] = fork();															/* fork() parent NUMBER_OF_SMOKERS times and log their pids in smokers[] */

		if (smokers[i] == 0) 															/* if i-th process is first parent's child process (= fork() returned 0) */
		{
			while(1)
			{
				switch(i)
				{
					case 0:																/* smoker-0 has tobacco and needs paper and matches */
						sem_wait(tobaccoSem);
						table[1]--;														/* take needed materials */
						table[2]--;
						printf("Took paper and matches. \n");
						sem_post(sellerSem);											/* signal seller */
						break;
					case 1:																/* smoker-1 has paper and needs tobacco and matches */
						sem_wait(paperSem);
						table[0]--;														/* take needed materials */
						table[2]--;
						printf("Took tobacco and matches. \n");
						sem_post(sellerSem);											/* signal seller */
						break;
					case 2:																/* smoker-2 has matches and needs tobacco and paper */
						sem_wait(matchSem);
						table[0]--;														/* take needed materials */
						table[1]--;
						printf("Took tobacco and paper. \n");
						sem_post(sellerSem);											/* signal seller */
						break;
					default:
						printf("Something went wrong.");
				}
			}
		}
	}


	/***** Seller's Code *****/


	printf("seller \n");

	while(1)																			/* seller goes into forever loop */
	{
		int choice = rand() % 3;														/* seller chooses 1 of 3 options randomly to put on the table */

		sem_wait(sellerSem);															/* seller checks if he is needed */

		switch(choice)
		{
			case 0:																		/* place tobacco and paper on table */
				table[0]++;
				table[1]++;
				printf("Placed tobacco and paper on the table. \n");
				sem_post(matchSem);														/* signal smoker who has matches */
				break;
			case 1:																		/* place tobacco and matches on table */
				table[0]++;
				table[2]++;
				printf("Placed tobacco and matches on the table. \n");
				sem_post(paperSem);														/* signal smoker who has paper */
				break;
			case 2:																		/* place paper and matches on table */
				table[1]++;
				table[2]++;
				printf("Placed paper and matches on the table. \n");
				sem_post(tobaccoSem);													/* signal smoker who has tobacco */
				break;
			default:
				printf("Something went wrong.\n");
		}

	}


	return (0);
}
