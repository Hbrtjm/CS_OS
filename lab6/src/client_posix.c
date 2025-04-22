#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "posix.h"

// All of the constants are defined in the posix.h

int main() 
{
	// Commenting this I realize the code smell from all of the magic numbers below... I will move them to header file (maybe)
	char  client_queue[64 + 9];
	char  buf[MSG_SIZE + 1]; // Because \0
	mqd_t server_q, client_q;
	int   id = -1;

	snprintf(client_queue, sizeof(client_queue), "/client_%d", getpid());
	mq_unlink(client_queue);

	client_q = mq_open(client_queue, O_CREAT | O_RDONLY, 0666, &attr);
	if (client_q == (mqd_t)-1) {
		perror("Client: mq_open client");
		exit(1);
	}

	// Write only, we just send them the INIT and Messages, anything else we receive with our own queue
	server_q = mq_open(SERVER_QUEUE, O_WRONLY);
	if (server_q == (mqd_t)-1) {
		perror("Client: mq_open server");
		mq_close(client_q);
		mq_unlink(client_queue);
		exit(1);
	}
	char init_msg[134];
	snprintf(init_msg, sizeof(init_msg), "INIT %s", client_queue);
	mq_send(server_q, init_msg, strlen(init_msg), 0);

	while ((mq_receive(client_q, buf, MSG_SIZE, NULL)) < 0) {
		if (errno == EAGAIN || errno == EINTR) {
			usleep(100000); // Some timeout, so that we don't get stuck on EAGAIN, since there might be no data available
			continue;
		}
		perror("Client: mq_receive ID");
		exit(1);
	}
	buf[MSG_SIZE + 18] = '\0'; // TODO - move to posix.h 
	if (sscanf(buf, "ID %d", &id) != 1) {
		fprintf(stderr, "Client: bad ID response: %s\n", buf);
		exit(1);
	}
	printf("Client: connected as #%d\n", id);
	// Child for listetning
	pid_t pid = fork();
	if (pid == 0) {
		while (1) {
			ssize_t r = mq_receive(client_q, buf, MSG_SIZE, NULL);
			if (r > 0) {
				buf[r] = '\0';
				printf("%s\n", buf);
			} else if (errno != EAGAIN && errno != EINTR) {
				perror("Client-child: mq_receive");
				break;
			}
		}
		exit(0);
	} else if (pid < 0) {
		perror("Client: fork");
		exit(1);
	}

	do {
		printf("Write a message: ");
		fgets(buf, sizeof(buf), stdin);
		buf[strcspn(buf, "\n")] = '\0';
		if (strcmp(buf, "exit") == 0) {
			mq_send(server_q, "EXIT", 4, 0);
			break;
		}
		char out[MSG_SIZE + 18]; // This generated warnings...
		snprintf(out, sizeof(out), "MSG %d %s", id, buf);
		if (mq_send(server_q, out, strlen(out), 0) == -1) {
			perror("Client: mq_send MSG");
		}
	} while(*buf); // While we read non-emtpy string, that is the first character of char* is not \0, we read new messages
	// Cleanup
	kill(pid, SIGTERM);
	waitpid(pid, NULL, 0);
	mq_close(client_q);
	mq_unlink(client_queue);
	mq_close(server_q);
	return 0;
}

