#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "systemv.h"

typedef struct {
	char jobs[MAX_MSG][MSG_SIZE];
	int in;
	int out;
} shared_memory_t;

sem semaphore;
shm shared_mem;
shared_memory_t *shm_ptr;

void set_doc(char *doc) {
	for (int i = 0; i < MSG_SIZE - 1; i++) {
		doc[i] = 'a' + (rand() % 26);
	}
	doc[MSG_SIZE - 1] = '\0';
}

void cleanup(int signo) {
	shmdt(shm_ptr);
	shmctl(shared_mem, IPC_RMID, NULL);
	semctl(semaphore, 0, IPC_RMID);
	exit(0);
}

void register_handlers() {
	signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);
}

void sem_op(int semid, int sem_num, int op) {
	struct sembuf operation;
	operation.sem_num = sem_num;
	operation.sem_op = op;
	operation.sem_flg = 0;
	if (semop(semid, &operation, 1) == -1) {
		perror("semop");
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s [# Clients]\n", argv[0]);
		return 1;
	}

	int clients = atoi(argv[1]);
	register_handlers();

	key_t key = ftok(".", PROJ_ID);
	if (key == -1) {
		perror("ftok");
		return 1;
	}

	shared_mem = shmget(key, sizeof(shared_memory_t), IPC_CREAT | 0666);
	if (shared_mem == -1) {
		perror("shmget");
		return 1;
	}

	shm_ptr = (shared_memory_t *) shmat(shared_mem, NULL, 0);
	if (shm_ptr == (void *) -1) {
		perror("shmat");
		return 1;
	}

	shm_ptr->in = 0;
	shm_ptr->out = 0;

	semaphore = semget(key, 2, IPC_CREAT | 0666);
	if (semaphore == -1) {
		perror("semget");
		return 1;
	}

	if (semctl(semaphore, 0, SETVAL, MAX_MSG) == -1) {
		perror("semctl empty");
		return 1;
	}
	if (semctl(semaphore, 1, SETVAL, 0) == -1) {
		perror("semctl full");
		return 1;
	}

	for (int i = 0; i < clients; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			srand(time(NULL) ^ getpid());
			while (1) {
				char doc[MSG_SIZE];
				set_doc(doc);

				sem_op(semaphore, 0, -1); 

				strcpy(shm_ptr->jobs[shm_ptr->in], doc);
				shm_ptr->in = (shm_ptr->in + 1) % MAX_MSG;

				sem_op(semaphore, 1, 1);  

				printf("[Client #%d PID %d] Sent job: %s\n", i, getpid(), doc);

				sleep(rand() % 5 + 1);
			}
			exit(0);
		}
	}

	for (int i = 0; i < clients; i++) {
		wait(NULL);
	}

	cleanup(0);

	return 0;
}

