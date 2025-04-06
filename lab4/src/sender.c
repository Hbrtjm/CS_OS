#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int confirmed = 0;

void confirm_handler(int signum) {
	confirmed = 1;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <PID catcher> <mode>\n", argv[0]);
		exit(1);
	}
	
	pid_t catcher_pid = atoi(argv[1]);
	int mode = atoi(argv[2]);
	
	struct sigaction sa;
	sa.sa_handler = confirm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
	
	union sigval value;
	value.sival_int = mode;
	
	if (sigqueue(catcher_pid, SIGUSR1, value) == -1) {
		perror("sigqueue");
		exit(1);
	}
	
	sigset_t mask;
	sigemptyset(&mask);
	while (!confirmed) {
		sigsuspend(&mask);
	}
	
	printf("Response received, program finished\n");
	return 0;
}

