#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <semaphore.h>
#include "headers/max.h"
#include "headers/aprint.h"


pid_t *pid;
int i,j,flag;
int child_status;
int N;
int *SA;

//Making the Bakery Algorithm
sem_t making_choice;
int *number;
int *choosing;

sem_t creating_processes;


int main() {

    /**** creating the N-sized array to store the processes needed ****/
    printf("How many processes do you want?\n");
    scanf("%d",&N);

    pid=(pid_t*)malloc(N*sizeof(pid_t));


    /***** creating the shared array *****/
    //SA=(int*)malloc(N*sizeof(int)); // Shared Array

    key_t shmkey; //shared memory key
    int shmid; //shared memory id

    shmkey = ftok("/Users/giannisloudaros/Library/Mobile Documents/com~apple~CloudDocs/Σχολή/5. Πέμπτο εξάμηνο /Λειτουργικά Συστήματα/Project/Project 1/Solution/Question_B", 1);
    shmid = shmget(shmkey, N*sizeof(int), 0644 | IPC_CREAT);
    SA = (int*)shmat(shmid,NULL,0);



    /***** creating the arrays needed for the Bakery Algorithm *****/

    key_t shmkeyn; //shared memory key
    int shmidn; //shared memory id

    shmkeyn = ftok("/Users/giannisloudaros/Library/Mobile Documents/com~apple~CloudDocs/Σχολή/5. Πέμπτο εξάμηνο /Λειτουργικά Συστήματα/Project/Project 1/Solution/Question_B", 2);
    shmidn = shmget(shmkeyn, N*sizeof(int), 0644 | IPC_CREAT);
    number = (int*)shmat(shmidn,NULL,0);
    //number=(int*)malloc(N*sizeof(int)); // Array for keeping the ticket



    key_t shmkeyc; //shared memory key
    int shmidc; //shared memory id

    shmkeyc = ftok("/Users/giannisloudaros/Library/Mobile Documents/com~apple~CloudDocs/Σχολή/5. Πέμπτο εξάμηνο /Λειτουργικά Συστήματα/Project/Project 1/Solution/Question_B", 3);
    shmidc = shmget(shmkeyc, N*sizeof(int), 0644 | IPC_CREAT);
    choosing = (int*)shmat(shmidc,NULL,0);
    //choosing=(int*)malloc(N*sizeof(int)); //Array for the choosing mechanism of the Bakery Algorithm



    //SA initialization
    for (i=0;i<N;i++) SA[i]=0;

    // Semaphore initialization
    sem_init(&making_choice, 0, 1);
    sem_init(&creating_processes, 0, 0);





    //creating the processes
    for (i = 0; i < N; i++)
    {
        pid[i] = fork();
        if (pid[i]==0) break;
    }

    //The last process tells the parent that it was created;
    if(pid[i]==0 & getpid()==getppid()+N) sem_post(&creating_processes);

    if (pid[i] == 0)
    /***** Child Processes *****/

    {
        // The process takes a ticket
        sem_wait(&making_choice);
        choosing[i]=1;
        sem_post(&making_choice);

        number[i]=max(number,N)+1;

        sem_wait(&making_choice);
        choosing[i]=0;
        sem_post(&making_choice);


        // The process waits for its tickets turn
        for (j=0;j<N;j++)
        {
            do {
                flag=1;
                sem_wait(&making_choice);
                if(number[j]!=0 && (number[i]<number[j])) flag=0;
                sem_post(&making_choice);
            } while (flag==0);
        }

        // The process uses the table
        for (j=0;j<N;j++) SA[j]+=i;

				aprint(SA,N);

    }

    else
    /***** Parent Process *****/

    {
        sem_wait(&creating_processes);//the parents waits for the last child to be created

        for (i = 0; i < N; i++)
        {
            pid_t wpid = waitpid(pid[i], &child_status, 0);
            if (WIFEXITED(child_status))
            {
                printf("Child%d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
            } else
            {
                printf("Child%d terminated abnormally\n", wpid);
            }
        }
    }
}
