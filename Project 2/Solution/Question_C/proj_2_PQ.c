#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>

#include <sys/types.h>													 						/* libraries for shared memory functions*/                                
#include <sys/shm.h>
#include <sys/ipc.h>

#include <fcntl.h>																				/* contains 0_CREAT code for sem_open */
#include <semaphore.h>																			/* library for POSIX semaphores */

int main() 
{
	pid_t qProcessId;																				/* Q process ID */		
								
	int child_status;																				/* child status for terminating Q (child) process */

	int sys;

		sem_t *sem = sem_open ("sem", O_CREAT, 0644, 0);											/* excersise only needs 2 semaphores */
		sem_t *sem2 = sem_open ("sem2", O_CREAT, 0644, 0);

	if (sem == SEM_FAILED )
	{
		printf("Semaphore initialization failed. Exiting... \n");
		exit(1);
	}

	qProcessId = fork();																			/* fork parent(process P) to create child (process Q) */																					

	if (qProcessId == 0) 																			/* if i-th process is first parent's child process (= fork() returned 0) */
	{		
		/**** Process Q Code ****/

		sem_wait(sem);					//E2 executes after E1
		system("echo This Is E2");
		system("echo This Is E3");
		sem_wait(sem);					//E6 executes after E5
		system("echo This Is E6");
		sem_post(sem2);					//E7 can execute
		sem_wait(sem);					//E4 executes after E7
		system("echo This Is E4");
		exit(0);

		/**** End Of Process Q Code ****/
	}

	/**** Process P Code ****/

	system("echo This Is E1");
	sem_post(sem);						//E2 can execute
	sys = system("echo This Is E5");
	sem_post(sem);						//E6 can execute
	sys = system("echo This Is E8");
	sys = system("echo This Is E9");	
	sem_wait(sem2);						//E7 executes after E6
	sys = system("echo This Is E7");
	sem_post(sem);						//E4 can execute

	pid_t wpid = waitpid(qProcessId, &child_status, 0);												/* terminate parent when child has terminated */

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