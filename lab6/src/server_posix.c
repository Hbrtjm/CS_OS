#include <mqueue.h> // For mq_open mq_send mq_receive and mq_close
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // For queue types (effectively file descriptors) 
#include <errno.h> // For proper error handling 
#include <unistd.h>
#include "posix.h"

// All of the constatns are defined in posix.h

int main() {
	// Define the main queue for handshakes, with read/write access for everyone, create it if does not exist, read only for the server, and with attributes defined in posix.h 
	mqd_t server_q;
	char  buf[MSG_SIZE + 1];
	mq_unlink(SERVER_QUEUE);
	server_q = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, 0666, &attr); // The attr, as well as all of the constants are defined in the posix.h header file
	if (server_q == (mqd_t)-1) {
		perror("Server: mq_open");
		exit(1);
	}
	printf("Server: listening on %s\n", SERVER_QUEUE);
	struct client clients[MAX_CLIENTS] = {0};

	while (1) {
		// As in documentation, as mentioned in system_v implementation, the server queue processes the first thing that comes its way, the queue was not set to O_NONBLOCK (!) 
		ssize_t bytes = mq_receive(server_q, buf, MSG_SIZE, NULL);
		if (bytes < 0) {
			if (errno == EINTR) continue;
			perror("Server: mq_receive");
			break;
		}
		buf[bytes] = '\0';
		if (strncmp(buf, "INIT ", 5) == 0) {
			char queue_name[64];
			sscanf(buf + 5, "%63s", queue_name);
			int slot = -1;
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (!clients[i].active) {
					slot = i;
					break;
				}
			}
			if (slot < 0) {
				fprintf(stderr, "Server: no free client slots\n");
				continue;
			}
			// This will be the client queue for to send the ID, which finishes the handshake
			mqd_t cq = mq_open(queue_name, O_WRONLY); 
			if (cq == (mqd_t)-1) {
				perror("Server: mq_open client queue");
				continue;
			}

			clients[slot].active = 1;
			clients[slot].mqd	= cq;
			strncpy(clients[slot].name, queue_name, sizeof(queue_name));
			char reply[36];
			snprintf(reply, sizeof(reply), "ID %d", slot);
			if (mq_send(cq, reply, strlen(reply), 0) == -1) {
				perror("Server: mq_send ID");
				mq_close(cq);
				clients[slot].active = 0;
				continue;
			}
			printf("Server: registered client %d (%s)\n", slot, queue_name);
		}
		else if (strncmp(buf, "MSG ", 4) == 0) {
			int sender;
			char text[MSG_SIZE];
			if (sscanf(buf + 4, "%d %1023[^\n]", &sender, text) < 2) {
				fprintf(stderr, "Server: malformed MSG\n");
				continue;
			}
			char out[MSG_SIZE + 18];
			snprintf(out, sizeof(out), "From %d: %s", sender, text);
			// Resend the received message back to every client except the sender
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].active && i != sender) {
					if (mq_send(clients[i].mqd, out, strlen(out), 0) == -1) {
						perror("Server: mq_send broadcast");
					}
				}
			}
		}
		else if (strcmp(buf, "EXIT") == 0) {
			printf("Server: shutting down\n");
			break;
		}
		else {
			fprintf(stderr, "Server: unknown command: %s\n", buf);
		}
	}
		
	// Cleanup
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].active) {
			mq_close(clients[i].mqd);
		}
	}
	mq_close(server_q);
	mq_unlink(SERVER_QUEUE);
	return 0;
}

