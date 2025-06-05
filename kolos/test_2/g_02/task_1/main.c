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
#include <errno.h>

#define FILE_NAME "common.txt"

// Declare System V semaphore identifier
int semid;

// Semaphore union for initialization
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void lock_semaphore(int semid) {
    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop - lock");
        exit(1);
    }
}

void unlock_semaphore(int semid) {
    struct sembuf sb = {0, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop - unlock");
        exit(1);
    }
}

int main(int argc, char** args){

    if(argc != 4){
        printf("Not a suitable number of program parameters\n");
        return(1);
    }

    // Create System V semaphore
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    union semun sem_union;
    sem_union.val = 1;
    if (semctl(semid, 0, SETVAL, sem_union) == -1) {
        perror("semctl - SETVAL");
        return 1;
    }

    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int parentLoopCounter = atoi(args[1]);
    int childLoopCounter = atoi(args[2]);

    char buf[100];
    pid_t childPid;
    int max_sleep_time = atoi(args[3]);

    int iex = 0;

    if ((childPid = fork())) {
        int status = 0;
        srand((unsigned)time(NULL) ^ getpid());

        while (parentLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            lock_semaphore(semid); // Lock semaphore

            sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            unlock_semaphore(semid); // Unlock semaphore
        }

        waitpid(childPid, &status, 0);
    } else {
        srand((unsigned)time(NULL) ^ getpid());

        while (childLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            lock_semaphore(semid); // Lock semaphore

            sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            unlock_semaphore(semid); // Unlock semaphore
        }
        _exit(0);
    }

    // Remove semaphore
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl - IPC_RMID");
        return 1;
    }

    close(fd);
    return 0;
}

