#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "systemv.h"

// All of the constants are defined in sytemv.h

int main() {
	char path[64], buf[MSG_SIZE];
	int client_qid, server_qid, id;
	pid_t child;

	// For each client create a queue	
	snprintf(path, sizeof(path), "/tmp/client_%d", getpid());
	int fd = open(path, O_CREAT|O_EXCL, 0666);
	if (fd < 0 && errno != EEXIST) { perror("open path"); exit(1); }
	if (fd >= 0) close(fd);
	
	key_t ckey = ftok(path, CLIENT_PROJ_ID);
	if (ckey == -1) { perror("ftok(client)"); exit(1); }
	client_qid = msgget(ckey, 0666 | IPC_CREAT);
	if (client_qid == -1) { perror("msgget(client)"); exit(1); }

	// Initial handshake with the server
	key_t skey = ftok(SERVER_KEY_PATH, SERVER_PROJ_ID);
	if (skey == -1) { perror("ftok(server)"); exit(1); }
	server_qid = msgget(skey, 0);
	if (server_qid == -1) { perror("msgget(server)"); exit(1); }

	// Handshake
	struct msg init = { .msg_type = 1 };
	snprintf(init.message, MSG_SIZE, "INIT %s", path);
	if (msgsnd(server_qid, &init, strlen(init.message)+1, 0) == -1)
		{ perror("msgsnd INIT"); exit(1); }

	struct msg reply;
	if (msgrcv(client_qid, &reply, MSG_SIZE, 2, 0) == -1)
		{ perror("msgrcv ID"); exit(1); }
	if (sscanf(reply.message, "ID %d", &id) != 1) {
		fprintf(stderr, "bad ID response: %s\n", reply.message);
		exit(1);
	}
	printf("Client: connected as #%d\n", id);

	// Fork a child for listening
	if ((child = fork()) == 0) {
		struct msg inc;
		while (1) {
			ssize_t r = msgrcv(client_qid, &inc, MSG_SIZE, 1, 0);
			if (r > 0) {
				inc.message[r] = '\0';
				printf("%s\n", inc.message);
			} else {
				perror("msgrcv child");
				break;
			}
		}
		_exit(0);
	} else if (child < 0) {
		perror("fork"); exit(1);
	}
	// Wait for the message, if the buffer is not empty, send a message
	do {	
		printf("Write a message: ");
		fgets(buf, sizeof(buf), stdin);
		buf[strcspn(buf, "\n")] = '\0';
		if (strcmp(buf, "exit") == 0) {
			struct msg ex = { .msg_type = 1 };
			strcpy(ex.message, "EXIT");
			msgsnd(server_qid, &ex, strlen(ex.message)+1, 0);
			break;
		}
		struct msg out = { .msg_type = 1 };
		snprintf(out.message, MSG_SIZE + 18 , "MSG %d %s", id, buf); // SEJM (I SENAT) like in the server
		if (msgsnd(server_qid, &out, strlen(out.message)+1, 0) == -1)
			perror("msgsnd MSG");
	} while(*buf);
	// KILL THE CHILD (cleanup)
	kill(child, SIGTERM);
	waitpid(child, NULL, 0);
	msgctl(client_qid, IPC_RMID, NULL);
	unlink(path);
	return 0;
}

