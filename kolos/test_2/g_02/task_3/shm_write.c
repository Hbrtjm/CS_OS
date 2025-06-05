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


// Create a unique key to identify the current System V inter-process communication (IPC)
key_t key= ftok(PATH,ID);

// Create a shared memory segment
// the returned value is the shared memory block identifier - i.e. shmid
int shmid;
if ((shmid= shmget(key, BUFFER_SIZE, IPC_CREAT | 0666) ==-1)
{
	fprintf(stderr, "Erorr while getting the shared mem seg:%s\n", strerror(errno));
	exit(1);
}

//Map shared memory segments to process address space
shmAddr= shmat(shmid, NULL, 0);
if(shmAddr== (void*)-1)
{
fprintf(stderr, ":%s\n", strerror(errno));
}

// Copy dataAddr to shmAddr
strcpy(shmAddr,dataAddr);

// detach process memory from shared memory
if ( shmdt(shmAddr) ==-1)
{
fprintf(stderr, "Error while detaching the shared mem :%s\n", strerror(errno));
}
return 0;
}

/*
gcc -o shm_write shm_write.c
*/
