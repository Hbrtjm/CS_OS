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
	void *shmAddr = NULL;
	char dataAddr[] = "Hello";
	
    // UtwĂłrz unikalny klucz do identyfikacji bieĹźÄcej komunikacji miÄdzy procesami (IPC) Systemu V	
	key_t key= ftok(PATH,ID);
	
	// UtwĂłrz segment pamiÄci wspĂłĹdzielonej
    // zwracanÄ wartoĹciÄ jest identyfikator bloku pamiÄci wspĂłĹdzielonej - czyli shmid
	int shmid;
	if ((shmid = shmget(key, BUFFER_SIZE, IPC_CREAT | 0666)) ==-1)
	{
		fprintf(stderr, "... :%s\n", strerror(errno));
		exit(1);
	}

	//Mapuj segmenty pamiÄci wspĂłĹdzielonej na przestrzeĹ adresowÄ procesu
	shmAddr = shmat(shmid, NULL, 0);
	if(shmAddr== (void*)-1)
	{
		fprintf(stderr, "Failed to attach the memory:%s\n", strerror(errno));
	}

	// Skopiuj dataAddr do shmAddr
	strcpy(shmAddr,dataAddr);

	// odĹÄcz pamiÄÄ procesu od pamiÄci wspĂłĹdzielonej
	if (shmdt(shmAddr) ==-1)
	{
		fprintf(stderr, "Error while detaching the shared memory :%s\n", strerror(errno));
	}
	return 0;
}

/*
gcc -o shm_write shm_write.c
*/