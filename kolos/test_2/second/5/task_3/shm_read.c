#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>

#define PATH "/tmp"
#define BUFFER_SIZE 1024
#define ID 0

int main(int argc, char const *argv[])
{
char * shmAddr;

// convert path and identifier to System V IPC key
key_t key= ftok (PATH,ID);

// Create shared memory segment
// the return value is the shared memory block identifier - i.e. shmid
int shmid;
if ((shmid = shmget(key,BUFFER_SIZE ,IPC_CREAT | 0666))  ==-1)
{
fprintf(stderr, "Error while getting the shared memory:%s\n", strerror(errno));
exit(1);
}

// Map shared memory segments to process address space
shmAddr = (char*)shmat(shmid, NULL, 0);
if(shmAddr==(void*)-1)
{
fprintf(stderr, "Error while getting teh memory address:%s\n", strerror(errno));
}

printf("%s\n",shmAddr);

// Disconnect
shmdt(shmAddr);
// Remove shared memory segment
shmctl(shmid,IPC_RMID,NULL);

return 0;
}

/*
gcc -o shm_read shm_read.c
*/
