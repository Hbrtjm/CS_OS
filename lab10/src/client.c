#include "client.h"
#include "common.h"
#include <fcntl.h>

static int signal_pipe[2] = {-1, -1};
static Client* global_client = NULL;

int client_stop(Client *client) 
{
	if (!client) return -1;
	
	pthread_mutex_lock(&client->state_mutex);
	client->is_running = 0;
	pthread_cond_signal(&client->shutdown_cond);
	pthread_mutex_unlock(&client->state_mutex);
	
	return 0;
}

void signal_handler(int sig) 
{
	char byte = 1;
	if (signal_pipe[1] != -1) {
		write(signal_pipe[1], &byte, 1);
	}
	if (global_client) {
		client_stop(global_client);
	}
}

int setup_signal_handling() 
{
	if (pipe(signal_pipe) == -1) {
		perror("Failed to create signal pipe");
		return -1;
	}

	// Get current flags first
	int flags = fcntl(signal_pipe[1], F_GETFL, 0);
	if (flags == -1) {
		perror("Failed to get pipe flags");
		return -1;
	}

	// Write end should be non-blocking
	if (fcntl(signal_pipe[1], F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("Failed to set pipe non-blocking");
		return -1;
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	return 0;
}

int check_signal() 
{
	char byte;
	return (read(signal_pipe[0], &byte, 1) > 0);
}

void client_cleanup(Client *client) 
{
	if (!client) return;

	client_disconnect(client);

	pthread_mutex_destroy(&client->send_mutex);
	pthread_mutex_destroy(&client->state_mutex);
	pthread_cond_destroy(&client->shutdown_cond);

	if (signal_pipe[0] != -1) {
		close(signal_pipe[0]);
		close(signal_pipe[1]);
		signal_pipe[0] = signal_pipe[1] = -1;
	}
}

void client_disconnect(Client *client) 
{
	if (!client) return;

	client_stop(client);

	if (client->is_connected && client->server_socket != -1) {
		close(client->server_socket);
		client->server_socket = -1;
		client->is_connected = 0;
	}

	if (client->receive_thread != 0) {
		pthread_join(client->receive_thread, NULL);
		client->receive_thread = 0;
	}
}

int client_connect(Client *client) 
{
	if (!client) {
		return -1;
	}

	struct sockaddr_in server_addr;
	client->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client->server_socket == -1) {
		perror("Socket creation failed");
		return -1;
	}

	// Initialize the server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(client->server_port);

	int result = inet_pton(AF_INET, client->server_ip, &server_addr.sin_addr);
	if (result <= 0) {
		if (result == -1) {
			perror("There was an error while converting the address");
		} else {
			fprintf(stderr, "Incorrect server address\n");
		}
		close(client->server_socket);
		return -1;
	}

	// Connect to server
	if (connect(client->server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Connection to server failed");
		close(client->server_socket);
		return -1;
	}

	printf("Connected to server %s:%d\n", client->server_ip, client->server_port);

	// Send registration message
	Message reg_msg = {0};
	reg_msg.type = MSG_LIST;
	strncpy(reg_msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	reg_msg.timestamp = time(NULL);
	
	if (send_message_protocol(client->server_socket, &reg_msg) == -1) {
		fprintf(stderr, "Failed to register with server\n");
		close(client->server_socket);
		return -1;
	}

	client->is_connected = 1;
	client->is_running = 1;

	return 0;
}

int client_start(Client* client) 
{
	if (!client->is_connected) {
		fprintf(stderr, "Client not connected to server\n");
		return -1;
	}
	
	// Setup signal handling
	if (setup_signal_handling() == -1) {
		return -1;
	}
	
	global_client = client;
	
	// Create thread for receiving messages
	if (pthread_create(&client->receive_thread, NULL, client_receive_messages, client) != 0) {
		perror("Failed to create receive thread");
		return -1;
	}
	
	printf("Client started. Type 'help' for commands.\n");
	
	// Run interactive mode in main thread
	client_run_interactive(client);
	
	return 0;
}

int client_init(Client* client, const char* client_id, const char* server_ip, int server_port) 
{
	if (!client || !client_id || !server_ip) {
		return -1;
	}

	if (strlen(client_id) >= MAX_CLIENT_NAME || !validate_client_id(client_id)) {
		fprintf(stderr, "Invalid client ID\n");
		return -1;
	}

	memset(client, 0, sizeof(Client));
	strncpy(client->client_id, client_id, MAX_CLIENT_NAME - 1);
	strncpy(client->server_ip, server_ip, 15);
	client->server_port = server_port;
	client->server_socket = -1;
	client->is_connected = 0;
	client->is_running = 0;
	
	if (pthread_mutex_init(&client->send_mutex, NULL) != 0) {
		perror("Failed to initialize send mutex");
		return -1;
	}

	if (pthread_mutex_init(&client->state_mutex, NULL) != 0) {
		perror("Failed to initialize state mutex");
		pthread_mutex_destroy(&client->send_mutex);
		return -1;
	}
	
	if (pthread_cond_init(&client->shutdown_cond, NULL) != 0) {
		perror("Failed to initialize condition variable for client shutdown");
		pthread_mutex_destroy(&client->send_mutex);
		pthread_mutex_destroy(&client->state_mutex);
		return -1;
	}
	
	return 0;
}

int client_send_list_request(Client* client) 
{
	if (!client) return -1;
	
	pthread_mutex_lock(&client->send_mutex);

	Message msg = {0};
	msg.type = MSG_LIST;
	strncpy(msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	msg.timestamp = time(NULL);

	int result = send_message_protocol(client->server_socket, &msg);
	pthread_mutex_unlock(&client->send_mutex);
	return result;
}

int client_send_2all(Client* client, const char* message) 
{
	if (!client || !message || strlen(message) == 0) {
		fprintf(stderr, "Invalid parameters for broadcast message\n");
		return -1;
	}

	pthread_mutex_lock(&client->send_mutex);
	printf("Getting the message ready\n");
	fflush(stdout);
	Message msg = {0};
	msg.type = MSG_2ALL;
	strncpy(msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	strncpy(msg.content, message, MAX_MESSAGE_SIZE - 1);
	msg.content_length = strlen(message);
	msg.timestamp = time(NULL);
	printf("Sending it\n");
	fflush(stdout);
	int result = send_message_protocol(client->server_socket, &msg);
	printf("Done sending\n");
	fflush(stdout);
	pthread_mutex_unlock(&client->send_mutex);
	return result;
}

int client_send_2one(Client* client, const char* recipient_id, const char* message) 
{
	if (!client || !recipient_id || !message || strlen(message) == 0) {
		fprintf(stderr, "Invalid parameters for private message\n");
		return -1;
	}

	pthread_mutex_lock(&client->send_mutex);

	Message msg = {0};
	msg.type = MSG_2ONE;
	strncpy(msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	strncpy(msg.recipient_id, recipient_id, MAX_CLIENT_NAME-1);
	strncpy(msg.content, message, MAX_MESSAGE_SIZE - 1);
	msg.content_length = strlen(message);
	msg.timestamp = time(NULL);

	int result = send_message_protocol(client->server_socket, &msg);

	pthread_mutex_unlock(&client->send_mutex);
	return result;
}

int client_send_stop(Client* client) 
{
	if (!client) return -1;
	
	pthread_mutex_lock(&client->send_mutex);

	Message msg = {0};
	msg.type = MSG_STOP;
	strncpy(msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	msg.timestamp = time(NULL);

	int result = send_message_protocol(client->server_socket, &msg);
	pthread_mutex_unlock(&client->send_mutex);
	return result;
}

int client_send_alive_response(Client* client) 
{
	if (!client) return -1;
	
	pthread_mutex_lock(&client->send_mutex);

	Message msg = {0};
	msg.type = MSG_ALIVE_RESPONSE;
	strncpy(msg.sender_id, client->client_id, MAX_CLIENT_NAME-1);
	msg.timestamp = time(NULL);

	int result = send_message_protocol(client->server_socket, &msg);
	pthread_mutex_unlock(&client->send_mutex);
	return result;
}

void* client_receive_messages(void* arg) 
{
	Client* client = (Client*)arg;
	Message msg;
	
	while (client->is_running) {
		int result = receive_message_protocol(client->server_socket, &msg);
		
		if (result <= 0) {
			if (result == 0) {
				printf("Server disconnected\n");
			} else {
				perror("Error receiving message: ");
			}
			client_stop(client);
			break;
		}
		
		handle_received_message(client, &msg);
	}
	
	return NULL;
}

void handle_received_message(Client* client, const Message* msg) 
{
	if (!client || !msg) return;
	
	switch (msg->type) {
		case MSG_CLIENT_LIST:
			display_client_list(msg);
			break;
		case MSG_BROADCAST:
			display_broadcast_message(msg);
			break;
		case MSG_PRIVATE:
			display_private_message(msg);
			break;
		case MSG_ALIVE:
			// Respond to alive check
			client_send_alive_response(client);
			break;
		case MSG_ERROR:
			printf("Server error: %s\n", msg->content);
			break;
		default:
			printf("Received unknown message type: %d\n", msg->type);
			break;
	}
}

void display_client_list(const Message* msg) 
{
	char timestamp[32];
	format_timestamp(msg->timestamp, timestamp, sizeof(timestamp));
	
	printf("\r[%s] Active clients:\n", timestamp);
	printf("%s\n\r > ", msg->content);
	fflush(stdout);
}

void display_broadcast_message(const Message* msg) 
{
	char timestamp[32];
	format_timestamp(msg->timestamp, timestamp, sizeof(timestamp));
	
	printf("\r[%s] Broadcast from %s: %s\n\r > ", timestamp, msg->sender_id, msg->content);
	fflush(stdout);
}

void display_private_message(const Message* msg) 
{
	char timestamp[32];
	format_timestamp(msg->timestamp, timestamp, sizeof(timestamp));
	
	printf("\r[%s] Private message from %s: %s\n\r > ", timestamp, msg->sender_id, msg->content);
	fflush(stdout);
}

void print_help() 
{
	printf("\nAvailable commands:\n");
	printf("  help					- Show this help message\n");
	printf("  list					- List all connected clients\n");
	printf("  2all <message>		  - Send message to all clients\n");
	printf("  2one <client_id> <msg>  - Send private message to specific client\n");
	printf("  quit, exit, stop		- Disconnect and exit\n");
	printf("\n");
}

void handle_user_input(Client* client, const char* input) {
	
	if (!client || !input) return;
	char *input_copy = (char*)malloc(MAX_MESSAGE_SIZE); 
	strcpy(input_copy,input);
	char* trimmed = trim_whitespace(input_copy);
	if (strlen(trimmed) == 0) {
		free(input_copy);
		return;
	}

	char* command = strtok(trimmed, " ");
	if (!command) {
		free(input_copy);
		return;
	}
	
	if (strcmp(command, "help") == 0) {
		print_help();
	}
	else if (strcmp(command, "list") == 0) {
		if (client_send_list_request(client) == -1) {
			printf("Failed to send list request\n");
		}
	}
	else if (strcmp(command, "2all") == 0) {
		char* message = strtok(NULL, "");
		if (message) {
			message = trim_whitespace(message);
			if (strlen(message) > 0) {
				if (client_send_2all(client, message) == -1) {
					printf("Failed to send broadcast message\n");
				}
			} else {
				printf("Message cannot be empty\n");
			}
		} else {
			printf("Usage: 2all <message>\n");
		}
	}
	else if (strcmp(command, "2one") == 0) {
		char* recipient = strtok(NULL, " ");
		char* message = strtok(NULL, "");
		
		if (recipient && message) {
			recipient = trim_whitespace(recipient);
			message = trim_whitespace(message);
			
			if (strlen(recipient) > 0 && strlen(message) > 0) {
				if (client_send_2one(client, recipient, message) == -1) {
					printf("Failed to send private message\n");
				}
			} else {
				printf("Recipient and message cannot be empty\n");
			}
		} else {
			printf("Usage: 2one <client_id> <message>\n");
		}
	}
	else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0 || strcmp(command, "stop") == 0) {
		printf("Disconnecting...\n");
		client_send_stop(client);
		client_stop(client);
	}
	else {
		printf("Unknown command: %s. Type 'help' for available commands.\n", command);
	}
	
	free(input_copy);
}

void client_run_interactive(Client* client) {
	char input[BUFFER_SIZE];
	
	while (client->is_running) {
		printf("\r > ");
		fflush(stdout);
		
		if (fgets(input, sizeof(input), stdin) == NULL) {
			if (feof(stdin)) {
				printf("\nEOF received, exiting...\n");
				break;
			}
			if (errno == EINTR) {
				// Interrupted by signal, check if we should continue
				if (check_signal()) {
					printf("\nInterrupt received, exiting...\n");
					break;
				}
				continue;
			}
			perror("Error reading input");
			break;
		}
		
		// Remove newline character
		size_t len = strlen(input);
		if (len > 0 && input[len-1] == '\n') {
			input[len-1] = '\0';
		}
		handle_user_input(client, input);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 4) {
		printf("Usage: %s <client_id> <server_ip> <server_port>\n", argv[0]);
		return 1;
	}
	
	// Create the client
	Client client;
	char *client_id = argv[1];
	char *server_ip = argv[2];
	int server_port = atoi(argv[3]);
	
	if (client_init(&client, client_id, server_ip, server_port) == -1) {
		fprintf(stderr, "Failed to initialize the client\n");
		return 2;
	}
	
	// Create the connection to the server
	if (client_connect(&client) == -1) {
		fprintf(stderr, "Failed to connect to the server\n");
		client_cleanup(&client);
		return 3;
	}
	
	client_start(&client);
	
	client_cleanup(&client);
	
	printf("Client shutdown complete\n");
	
	return 0;
}
