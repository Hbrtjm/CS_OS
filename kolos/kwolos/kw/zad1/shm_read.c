#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>

#define PATH        "/tmp"
#define BUFFER_SIZE 1024
#define ID          0

int main(int argc, char const *argv[])
{
    char *shmAddr;
    key_t key;

    key = ftok(PATH, ID);
    if (key == (key_t)-1) {
        fprintf(stderr, "ftok error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int shmid;
    if ((shmid = shmget(key, BUFFER_SIZE, IPC_CREAT | 0666)) == -1) {
        fprintf(stderr, "shmget error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    shmAddr = shmat(shmid, NULL, 0);
    if (shmAddr == (void *)-1) {
        fprintf(stderr, "shmat error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Read from shared memory: \"%s\"\n", shmAddr);

    if (shmdt(shmAddr) == -1) {
        fprintf(stderr, "shmdt error: %s\n", strerror(errno));
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

/*
gcc -o shm_read shm_read.c
*/
