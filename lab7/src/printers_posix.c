#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "posix.h"

sem_t *sem;
shm_t sharedfd;
shared_memory_t *shared_mem;
int child;

void print_debug(char *message) {
	if(DEBUG) {
		printf("%s\n", message);
		fflush(stdout);
	}
}

void print_msg(char *doc, int printer_id, int timeout, char *finished_message)
{
	for (int i = 0; i < MSG_SIZE - 1; i++)
	{
		printf("\033[%d;1H", printer_id + 1);

		printf("\033[K");
	printf("[Printer\t#%d\tPID\t%d]:\t" , printer_id, getpid()); 
		for (int j = 0; j <= i; j++)
		{
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
	for(int i = 0; i < child; i++) wait(NULL);
	sem_close(sem);
	sem_unlink(SEM_NAME);
	munmap(shared_mem, sizeof(shared_memory_t));
	shm_unlink(SHARED_NAME);
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


int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Usage: %s [# Printers]\n", argv[0]);
		return 1;
	}
	cleanup(0);
	
	int printers = atoi(argv[1]);
	child = printers;
	register_handlers();
	printf("\033[2J\033[H");

	sharedfd = shm_open(SHARED_NAME, O_CREAT | O_RDWR, 0666);
	if(sharedfd == -1) {
		perror("shm_open");
		return 1;
	}

	if(ftruncate(sharedfd, sizeof(shared_memory_t)) == -1) {
		perror("ftruncate");
		return 1;
	}

	shared_mem = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
	if(shared_mem == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	shared_mem->in = 0;
	shared_mem->out = 0;

	sem = sem_open(SEM_NAME, O_CREAT, 0666, MAX_MESSAGES);
	if(sem == SEM_FAILED) {
		perror("sem_open");
		return 1;
	}
	for(int i = 0; i < printers; i++) {
		pid_t pid = fork();
		print_msg("xxxxxxxxxx", i, 0, "Waiting");
		if(pid == 0) {
			unregister_handlers();
			while(1) {
				if(shared_mem->out != shared_mem->in) {
					char doc[MSG_SIZE];
					strncpy(doc, shared_mem->jobs[shared_mem->out], MSG_SIZE);
					shared_mem->out = (shared_mem->out + 1) % MAX_MESSAGES;
					print_msg(doc, i, PRINT_TIMEOUT, FINISHED_MESSAGE);
					sem_post(sem); 
				} else {
					sleep(1);
				}
			}
		}
	}

	for(int i = 0; i < printers; i++) wait(NULL);
	cleanup(0);
	return 0;
}
