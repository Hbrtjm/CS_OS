#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
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
shm shared_memory;
shared_memory_t *shm_ptr;
int child = 0;
int printers = 0;

#define DEBUG 1

void print_debug(char *message) {
	if (DEBUG) {
		printf("%s\n", message);
		fflush(stdout);
	}
}

void print_msg(char *doc, int printer_id, int timeout, char *finished_message) {
	for (int i = 0; i < MSG_SIZE - 1; i++) {
		printf("\033[%d;1H", printer_id + 1);
		printf("\033[K");
		printf("[Printer\t#%d\tPID\t%d]:\t", printer_id, getpid());
		for (int j = 0; j <= i; j++) {
			putchar(doc[j]);
		}
		fflush(stdout);
		sleep(timeout);
	}

	printf("\033[%d;1H", printer_id + 1);
	printf("\033[K");
	printf("[Printer\t#%d\tPID\t%d]:\t[%s]\n", printer_id, getpid(), finished_message);
	fflush(stdout);
}

void cleanup(int signo) {
	for (int i = 0; i < child; i++) wait(NULL);

	print_debug("Detaching shared memory...");
	shmdt(shm_ptr);

	print_debug("Removing shared memory...");
	shmctl(shared_memory, IPC_RMID, NULL);

	print_debug("Removing semaphore...");
	semctl(semaphore, 0, IPC_RMID);

	print_debug("Resources cleaned up.");
}

void cleanup_exit(int signo) {
	cleanup(signo);
	exit(0);
}

void register_handlers() {
	signal(SIGINT, cleanup_exit);
	signal(SIGTERM, cleanup_exit);
}

void unregister_handlers() {
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
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
		printf("Usage: %s [# Printers]\n", argv[0]);
		return 1;
	}

	printers = atoi(argv[1]);
	child = printers;
	register_handlers();

	printf("\033[2J\033[H");

	key_t key = ftok(".", PROJ_ID);
	if (key == -1) {
		perror("ftok");
		return 1;
	}

	semaphore = semget(key, 2, IPC_CREAT | 0666);
	if (semaphore == -1) {
		perror("semget");
		return 1;
	}

	shared_memory = shmget(key, sizeof(shared_memory_t), IPC_CREAT | 0666);
	if (shared_memory == -1) {
		perror("shmget");
		return 1;
	}

	shm_ptr = (shared_memory_t *) shmat(shared_memory, NULL, 0);
	if (shm_ptr == (void *) -1) {
		perror("shmat");
		return 1;
	}

	for (int i = 0; i < printers; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			srand(time(NULL) ^ getpid());

			while (1) {
				sem_op(semaphore, 1, -1);
				char doc[MSG_SIZE];
				strcpy(doc, shm_ptr->jobs[shm_ptr->out]);
				shm_ptr->out = (shm_ptr->out + 1) % MAX_MSG;

				sem_op(semaphore, 0, 1);

				print_msg(doc, i, 1, "Done printing.");
			}

			exit(0);
		} else if (pid == -1) {
			perror("Error while forking");
			exit(1);
		}
	}

	for (int i = 0; i < printers; i++) {
		wait(NULL);
	}

	cleanup(0);
	return 0;
}

