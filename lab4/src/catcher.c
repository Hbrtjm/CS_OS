#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void reaction(int signum)
{ 
	printf("CTRL+C was pressed\n"); 
}

int request_count = 0;
int current_mode = 0;
int ctrl_c_mode = 0;
pid_t sender_pid = 0;
int print_numbers = 0;

void handle_sigusr1(int sig, siginfo_t *info, void *context) {
	if (info == NULL)
		return;

	sender_pid = info->si_pid;
	int mode = info->si_value.sival_int;	
	request_count++;
	current_mode = mode;
	
	switch (mode) {
		case 1:
			printf("Mode 1: Number of requests: %d\n", request_count);
			ctrl_c_mode = 0;
			break;
		case 2:
			printf("Mode 2: Printing a number every second...\n");
			print_numbers = 1;
			ctrl_c_mode = 0;
			break;
		case 3:
			printf("Mode 3: Ignoring Ctrl+C\n");
			ctrl_c_mode = 1;
			signal(SIGINT, SIG_IGN);
			break;
		case 4:
			printf("Mode 4: Reacting to Ctrl+C\n");
			ctrl_c_mode = 2;
			signal(SIGINT, reaction);
			break;
		case 5:
			printf("Mode 5: Killing catcher process\n");
			ctrl_c_mode = 0;
			exit(0);
		default:
			ctrl_c_mode = 0;
			printf("Mode not defined: %d\n", mode);
	}
	if(!ctrl_c_mode)
	{
		signal(SIGINT, SIG_DFL);
	}
	union sigval val;
	val.sival_int = 0;
	sigqueue(sender_pid, SIGUSR1, val);
}

int main() {
	printf("Catcher PID: %d\n", getpid());
	
	struct sigaction handler;
	handler.sa_sigaction = handle_sigusr1;
	sigemptyset(&handler.sa_mask);
	handler.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &handler, NULL);
	sigset_t mask;
	sigemptyset(&mask);
	
	while (1) {
		if (print_numbers && current_mode == 2) {
			static int num = 1;
			printf("Number: %d\n", num++);
			sleep(1);
		} else {
			sigsuspend(&mask);
		}
	}
	return 0;
}

