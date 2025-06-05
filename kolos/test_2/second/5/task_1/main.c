#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "/control_sem"
#define PROJ_ID 0

   union semun {
       int              val;    /* Value for SETVAL */
   };

typedef int semt;

const int semnum = 0;

//    .buf = NULL;   
//    .array = NULL; 
//    .__buf = NULL; 
//};


// Declare System V semaphore identifier variable
// Used a custom semaphore type, but it's just an int
semt semid;

int main(int argc, char** args){

union semun zero; 
zero.val = 0;
union semun one;
one.val = 1;
   if(argc != 4){
      printf("Not a suitable number of program parameters\n");
      return(1);
   }

   /**************************************************
   Create System V semaphore
   Initial value to 1 (binary semaphore)
   ***************************************************/
   semid = semget(IPC_PRIVATE, 1, IPC_CREAT);
	if (semid == -1)
	{
		fprintf(stderr, "Errod while getting a semaphore");
		return 1;
	}	
   int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC , 0644);

   int parentLoopCounter = atoi(args[1]);
   int childLoopCounter = atoi(args[2]);

   char buf[100];
   pid_t childPid;
   int max_sleep_time = atoi(args[3]);

   // Shared loop variable to illustrate race condition if xxxxx not used
   // Declare shared xxxxx variable for demonstration purpose
   // Example: xxxx ii = 0;
	char ii = '\0';
   if ((childPid = fork())) {
      int status = 0;
      srand((unsigned)time(0));
	int res;
      while (parentLoopCounter--) {
         int s = rand() % max_sleep_time + 1;
         sleep(s);

         /*****************************************
         Lock semaphore before entering critical section
         *****************************************/
	res = semctl(semid, semnum, GETVAL,NULL);
	 while(res== 0)
		{
			res = semctl(semid, semnum, GETVAL, NULL);
			if(res == -1)
			{
				fprintf(stderr, "Error while reading sem value");
				exit(1);
			}
		}
		 semctl(semid, semnum, SETVAL, zero); // Set to 0

         sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
         write(fd, buf, strlen(buf));
         write(1, buf, strlen(buf));
	 ii = 'r';
	 semctl(semid, semnum, SETVAL, one); // Set to 1
         /*********************************
         TODO: Unlock semaphore after critical section
         *********************************/
      }

      waitpid(childPid, &status, 0);
   } else {
      srand((unsigned)time(0));
	int res;
      while (childLoopCounter--) {
         int s = rand() % max_sleep_time + 1;
         sleep(s);

         /*****************************************
         TODO: Lock semaphore before entering critical section
         *****************************************/
	res = semctl(semid, semnum, GETVAL,NULL);
	 while(res== 0)
		{
			res = semctl(semid, semnum, GETVAL, NULL);
			if(res == -1)
			{
				fprintf(stderr, "Error while reading sem value");
				exit(1);
			}
		}
		 semctl(semid, semnum, SETVAL, zero); // Set to 0
         ii = 'c';
		 sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
         write(fd, buf, strlen(buf));
         write(1, buf, strlen(buf));
	semctl(semid, semnum, SETVAL, one); // Set to 1
         /*********************************
         TODO: Unlock semaphore after critical section
         *********************************/
      }
      _exit(0);
   }

   /*****************************
   TODO: Remove semaphore
   ******************************/
	semctl(semid, semnum, IPC_RMID, NULL);
   close(fd);
   return 0;
}
