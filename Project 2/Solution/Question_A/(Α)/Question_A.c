#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include "headers/tree.h"


pid_t* pid;
int N,i;
int* p;
sem_t mutex;
sem_t calculation_done;

//Μεταβλητές που θα χρησιμοποιούμε για τις μετρήσεις μας
clock_t start, mend;
double time_taken;
double time_taken2;


int main() {


  //creating the N-sized array to store the processes needed
  printf("How many processes do you want?\n");
  scanf("%d",&N);

  pid=(pid_t*)malloc(N*sizeof(pid_t));



/****** Creating the B+ shared tree and the shared variable p ******/
  node** root;

  key_t shmkey; //shared memory key
  int shmid; //shared memory id

  shmkey = ftok("/Users/giannisloudaros/Library/Mobile Documents/com~apple~CloudDocs/Σχολή/5. Πέμπτο εξάμηνο /Λειτουργικά Συστήματα/Project/Project 2/Solution/Question_A", 1);
  shmid = shmget(shmkey, sizeof(node**), 0644 | IPC_CREAT);
  root = (node**)shmat(shmid,NULL,0);

  *root = sampleTree(N);


  key_t shmkeyp; //shared memory key
  int shmidp; //shared memory id

  shmkeyp = ftok("/Users/giannisloudaros/Library/Mobile Documents/com~apple~CloudDocs/Σχολή/5. Πέμπτο εξάμηνο /Λειτουργικά Συστήματα/Project/Project 2/Solution/Question_A", 2);
  shmidp = shmget(shmkeyp, sizeof(int*), 0644 | IPC_CREAT);
  p = (int*)shmat(shmid,NULL,0);
  *p=0;

  // Semaphore initialization
  sem_init(&mutex, 0, 1);
  sem_init(&calculation_done, 0, 0);





  start = clock();//starting the timer

  /***** creating the Processes *****/
  for ( i = 0; i < N; i++)
  {
      pid[i] = fork();
      if (pid[i]==0) break;
  }


  if (pid[i] == 0)
  /***** Child Processes *****/

  {
    sem_wait(&mutex);
    *p += findAndGet(*root, i,true);
    sem_post(&mutex);
    //The last process tells the parent that it did the addition;
    if( (pid[i]==0) & (getpid()== (getppid()+N)) ) sem_post(&calculation_done);
  }

  else
  /***** Parent Process *****/

  {
      sem_wait(&calculation_done);//the parents waits for the last child to do the addition
      mend = clock();

      time_taken = ((double) (mend - start)) / CLOCKS_PER_SEC;

      printf("\n Το αποτέλεσμα είναι: %d",*p);

      printf("\nΟ υπολογισμός χρειάστηκε %lf δευτερόλεπτα\n",time_taken);


  }



return 0;
}
