/**************************************************************************************************************************************************************************/
//	This C program utilizes semaphores to allow the 2 communicating processes to execute each of their commands with a given order.
//	At first the parent process (P) creates 1 separate process (Q) using fork().
//  Each process' command needs to be executed with the following rules in mind (these are the only rules that need to be controlled with semaphores):
//	
//	E1 executes before E2
//	E5 ececutes before E6	
//	E6 executes before E7
//	E7 executes before E4
//
//	For this to be achieved, after analyzing the restrictions, we will need 2 semaphores, 
//  one for the parent process (P) to signal the child process (Q) when it can continue and vice versa.
//
//	The semaphores where implemented using the POSIX Semaphores package which is supported by Linux.
//	The code was written and tested on Ubuntu 20.04 with GNU C/C++ Compiler 9.3.0
//
/**************************************************************************************************************************************************************************/

#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>

#include <fcntl.h>																					/* contains 0_CREAT code for sem_open */
#include <semaphore.h>																				/* library for POSIX semaphores */

int main() 
{
	pid_t qProcessId;																				/* Q process ID */		
								
	int child_status;																				/* child status for terminating Q (child) process */


		sem_t *sem = sem_open ("sem", O_CREAT, 0644, 0);											/* program only requires 2 semaphores, initialized as 0 */
		sem_t *sem2 = sem_open ("sem2", O_CREAT, 0644, 0);

	if (sem == SEM_FAILED )																			/* check if semaphore initialization was successful */
	{
		printf("Semaphore initialization failed. Exiting... \n");
		exit(1);
	}

	qProcessId = fork();																			/* fork parent(process P) to create child (process Q) */																					

	if (qProcessId == 0) 																			/* if process is the child-process (= fork() returned 0) */
	{		
		/**** Process Q Code ****/

		sem_wait(sem);					//E2 will execute after E1
		system("echo This Is E2");
		system("echo This Is E3");
		sem_wait(sem);					//E6 will execute after E5
		system("echo This Is E6");
		sem_post(sem2);					//E7 can now execute
		sem_wait(sem);					//E4 will execute after E7
		system("echo This Is E4");
		exit(0);

		/**** End Of Process Q Code ****/
	}

	/**** Process P Code ****/

	system("echo This Is E1");
	sem_post(sem);						//E2 can now execute
	system("echo This Is E5");
	sem_post(sem);						//E6 can now execute
	system("echo This Is E8");
	system("echo This Is E9");	
	sem_wait(sem2);						//E7 will execute after E6
	system("echo This Is E7");
	sem_post(sem);						//E4 can now execute

	pid_t wpid = waitpid(qProcessId, &child_status, 0);												/* wait to terminate parent after child has terminated */

	if (WIFEXITED(child_status)) 															 		/* WIFEXITED returns true if child exited normally */
	{
		printf("Child%d terminated with return value %d\n", wpid, WEXITSTATUS(child_status));		/* prints terminated child's id and its exit status */
	}
	else
	{
		printf("Child%d terminated abnormally\n", wpid);
	}

	return (0);

	/**** End Of Process Q Code ****/
}