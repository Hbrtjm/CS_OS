#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "systemv.h"

// All of the constants are defined in sytemv.h

int main() {
	// Create the main queue key for handshakes with cleints
	key_t skey = ftok(SERVER_KEY_PATH, SERVER_PROJ_ID);
	if (skey == -1) { perror("ftok(server)"); exit(1); }

	// Queue with aforementioned key
	int server_qid = msgget(skey, 0666 | IPC_CREAT);
	if (server_qid == -1) { perror("msgget(server)"); exit(1); }
	printf("Server: listening on queue %d\n", server_qid);

	struct client clients[MAX_CLIENTS] = {0};
	while (1) {
		struct msg in;
		// This is what I read from documentation, but I think it might as well be an int, from what I read it's bound by system-dependednt SSIZE_MAX, but a normal int or long would store it, at worst overflow 
		ssize_t r = msgrcv(server_qid, &in, MSG_SIZE, 0, 0);
		if (r < 0) {
			if (errno == EINTR) continue;
			perror("Server: msgrcv");
			break;
		}
		in.message[r] = '\0';
		// Client sent in a INIT message, connect them
		if (strncmp(in.message, "INIT ", 5) == 0) {
			char path[64];
			if (sscanf(in.message + 5, "%63s", path) != 1) {
				fprintf(stderr, "Server: malformed INIT\n");
				continue;
			}
			int slot = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
				if (!clients[i].active) { slot = i; break; }
			if (slot < 0) {
				fprintf(stderr, "Server: no slots\n");
				continue;
			}
			// Create a client message queue key 
			key_t ckey = ftok(path, CLIENT_PROJ_ID);
			if (ckey == -1) { perror("Server: ftok(client)"); continue; }
			int cqid = msgget(ckey, 0);
			if (cqid == -1) { perror("Server: msgget(client)"); continue; }

			// And reserve a spot for the new client
			clients[slot].active = 1;
			clients[slot].qid	= cqid;

			// Confirmation, ending handshake
			struct msg out = { .msg_type = 2 };
			snprintf(out.message, MSG_SIZE, "ID %d", slot);
			if (msgsnd(cqid, &out, strlen(out.message)+1, 0) == -1)
				perror("Server: msgsnd ID");
			else
				printf("Server: client %d registered (qid=%d)\n", slot, cqid);
		}
		else if (strncmp(in.message, "MSG ", 4) == 0) {
			// Message form the client
			int from;
			char text[MSG_SIZE];
			// Sscanf can be dangerous due to fuzzing, but for now we ignore, it's not cybersecurity course, it's computer science
			if (sscanf(in.message + 4, "%d %[^\n]", &from, text) != 2) {
				fprintf(stderr, "Server: bad MSG\n");
				continue;
			}
			char broadcast[MSG_SIZE];
			snprintf(broadcast, MSG_SIZE + 18, "From %d: %s", from, text); // Warning - SKILL ISSUE

			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].active && i != from) {
					struct msg out = { .msg_type = 1 };
					strncpy(out.message, broadcast, MSG_SIZE);
					if (msgsnd(clients[i].qid, &out, strlen(out.message)+1, 0) == -1)
						perror("Server: broadcast");
				}
			}
		}
		// Any client can kill the server... Reminds me of aireplay-ng --deauth or just any DOS
		else if (strcmp(in.message, "EXIT") == 0) {
			printf("Server: EXIT received, shutting down\n");
			break;
		}
		else {
			fprintf(stderr, "Server: unknown cmd '%s'\n", in.message);
		}
	}

	msgctl(server_qid, IPC_RMID, NULL);
	return 0;
}

