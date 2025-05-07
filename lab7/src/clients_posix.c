#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "posix.h"

sem_t *sem;
shm_t sharedfd;
shared_memory_t *shared_mem;

void set_doc(char *doc) {
	for(int i = 0; i < MSG_SIZE - 1; i++) {
		doc[i] = 'a' + (rand() % 26);
	}
	doc[MSG_SIZE - 1] = '\0';
}

void cleanup(int signo) {
	munmap(shared_mem, sizeof(shared_memory_t));
	close(sharedfd);
	sem_close(sem);
	exit(0);
}

void register_handlers() {
	signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Usage: %s [# Clients]\n", argv[0]);
		return 1;
	}

	int clients = atoi(argv[1]);
	register_handlers();

	sharedfd = shm_open(SHARED_NAME, O_RDWR, 0666);
	if(sharedfd == -1) {
		perror("shm_open");
		return 1;
	}

	shared_mem = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
	if(shared_mem == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	sem = sem_open(SEM_NAME, O_RDWR);
	if(sem == SEM_FAILED) {
		perror("sem_open");
		return 1;
	}

	for(int i = 0; i < clients; i++) {
		pid_t pid = fork();
		if(pid == 0) {
			srand(time(NULL) ^ getpid());
			while (1) {
				char doc[MSG_SIZE];
				set_doc(doc);

				sem_wait(sem);
				strcpy(shared_mem->jobs[shared_mem->in], doc);
				shared_mem->in = (shared_mem->in + 1) % MAX_MESSAGES;
				printf("[Client #%d PID %d] Sent job: %s\n", i, getpid(), doc);

				sleep(rand() % 5 + 1);
			}
			exit(0);
		}
	}

	for(int i = 0; i < clients; i++) wait(NULL);

	munmap(shared_mem, sizeof(shared_memory_t));
	close(sharedfd);
	sem_close(sem);

	return 0;
}

