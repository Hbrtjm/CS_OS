#include "server.h"
#include "common.h"

static Server* g_server = NULL;

void signal_handler(int signo) 
{
	printf("\nReceived signal %d. Shutting down server...\n", signo);
	if (g_server) 
	{
		server_stop(g_server);
	}
}

// Initialize server structure
int server_init(Server* server, int port)
{
	if (!server || port < PORT_MIN || port > PORT_MAX)
{
		fprintf(stderr, "Invalid server pointer or port number\n");
		return -1;
	}

	memset(server, 0, sizeof(Server));
	server->port = port;
	server->client_count = 0;
	server->is_running = 0;
	server->server_socket = -1;

	for (int i = 0; i < MAX_CLIENTS; i++)
{
		server->clients[i].socket_fd = -1;
		server->clients[i].is_active = 0;
		server->clients[i].inactive_count = 0;
		memset(server->clients[i].client_id, 0, MAX_CLIENT_NAME);
	}

	if (pthread_mutex_init(&server->clients_mutex, NULL) != 0) 
	{
		perror("Mutex initialization failed");
		return -1;
	}

	server->server_socket = create_server_socket(port);
	if (server->server_socket == -1)
{
		pthread_mutex_destroy(&server->clients_mutex);
		return -1;
	}

	printf("Server initialized on port %d\n", port);
	return 0;
}

void server_start(Server* server)
{
	if (!server)
{
		return;
	}

	g_server = server;
	server->is_running = 1;

	// Set up signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Start keepalive monitor thread
	if (pthread_create(&server->keepalive_thread, NULL, keepalive_monitor, server) != 0)
{
		perror("Failed to create keepalive thread");
		server_stop(server);
		return;
	}

	printf("Server started. Listening for connections...\n");

	// Main server loop
	while (server->is_running)
{
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		
		int client_socket = accept(server->server_socket, (struct sockaddr*)&client_addr, &addr_len);
		
		if (client_socket == -1)
{
			if (errno == EINTR && !server->is_running)
{
				break; // Interrupted by signal during shutdown
			}
			if (errno != EINTR)
{
				perror("Accept failed");
			}
			continue;
		}

		// Configure client socket
		if (configure_socket(client_socket) == -1)
{
			close(client_socket);
			continue;
		}

		printf("New connection from %s:%d\n", 
			   inet_ntoa(client_addr.sin_addr), 
			   ntohs(client_addr.sin_port));

		// Wait for client identification message
		Message id_msg;
		int result = receive_message_protocol(client_socket, &id_msg);
		
		if (result <= 0)
{
			printf("Failed to receive client identification\n");
			close(client_socket);
			continue;
		}

		// Validate client ID
		if (!validate_client_id(id_msg.sender_id))
{
			printf("Invalid client ID: '%s'\n", id_msg.sender_id);
			Message err_msg = create_message(MSG_ERROR, "SERVER", id_msg.sender_id, 
										   "Invalid client ID format");
			send_message_protocol(client_socket, &err_msg);
			close(client_socket);
			continue;
		}

		// Check if client ID already exists
		pthread_mutex_lock(&server->clients_mutex);
		int existing_client = find_client_by_id(server, id_msg.sender_id);
		if (existing_client != -1)
{
			pthread_mutex_unlock(&server->clients_mutex);
			printf("Client ID '%s' already exists\n", id_msg.sender_id);
			Message err_msg = create_message(MSG_ERROR, "SERVER", id_msg.sender_id, 
										   "Client ID already in use");
			send_message_protocol(client_socket, &err_msg);
			close(client_socket);
			continue;
		}

		// Add client to server
		int client_index = add_client(server, client_socket, client_addr, id_msg.sender_id);
		if (client_index == -1)
{
			pthread_mutex_unlock(&server->clients_mutex);
			printf("Failed to add client or server full\n");
			Message err_msg = create_message(MSG_ERROR, "SERVER", id_msg.sender_id, 
										   "Server full or internal error");
			send_message_protocol(client_socket, &err_msg);
			close(client_socket);
			continue;
		}

		// Create thread to handle client
		if (pthread_create(&server->clients[client_index].thread_id, NULL, 
						  handle_client, &server->clients[client_index]) != 0)
{
			perror("Failed to create client thread");
			remove_client(server, client_index);
			pthread_mutex_unlock(&server->clients_mutex);
			close(client_socket);
			continue;
		}

		pthread_mutex_unlock(&server->clients_mutex);

		// Send welcome message
		Message welcome_msg = create_message(MSG_BROADCAST, "SERVER", "", 
										   "Welcome to the chat server!");
		send_message_protocol(client_socket, &welcome_msg);

		printf("Client '%s' connected successfully\n", id_msg.sender_id);
	}

	printf("Server main loop ended\n");
}

// Stop the server
void server_stop(Server* server)
{
	if (!server || !server->is_running)
{
		return;
	}

	printf("Stopping server...\n");
	server->is_running = 0;

	// Close server socket to break accept loop
	if (server->server_socket != -1)
{
		close(server->server_socket);
		server->server_socket = -1;
	}

	// Send stop messages to all clients and close connections
	pthread_mutex_lock(&server->clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; i++)
{
		if (server->clients[i].is_active)
{
			Message stop_msg = create_message(MSG_STOP, "SERVER", 
											server->clients[i].client_id, 
											"Server shutting down");
			send_message_protocol(server->clients[i].socket_fd, &stop_msg);
			close(server->clients[i].socket_fd);
			server->clients[i].socket_fd = -1;
			server->clients[i].is_active = 0;
		}
	}
	pthread_mutex_unlock(&server->clients_mutex);

	// Wait for keepalive thread to finish
	if (server->keepalive_thread != 0)
{
		pthread_join(server->keepalive_thread, NULL);
	}

	printf("Server stopped\n");
}

// Cleanup server resources
void server_cleanup(Server* server)
{
	if (!server)
{
		return;
	}

	server_stop(server);

	// Wait for all client threads to finish
	pthread_mutex_lock(&server->clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; i++)
{
		if (server->clients[i].thread_id != 0)
{
			pthread_join(server->clients[i].thread_id, NULL);
			server->clients[i].thread_id = 0;
		}
	}
	pthread_mutex_unlock(&server->clients_mutex);

	// Cleanup mutex
	pthread_mutex_destroy(&server->clients_mutex);

	// Close server socket if still open
	safe_close_socket(&server->server_socket);

	printf("Server cleanup completed\n");
}

// Add a new client to the server
int add_client(Server* server, int client_socket, struct sockaddr_in client_addr, const char* client_id)
{
	if (!server || client_socket < 0 || !client_id)
{
		return -1;
	}

	// Find empty slot
	for (int i = 0; i < MAX_CLIENTS; i++)
{
		if (!server->clients[i].is_active)
{
			server->clients[i].socket_fd = client_socket;
			server->clients[i].addr = client_addr;
			server->clients[i].last_seen = time(NULL);
			server->clients[i].inactive_count = 0;
			server->clients[i].is_active = 1;
			server->clients[i].thread_id = 0;
			safe_strcpy(server->clients[i].client_id, client_id, MAX_CLIENT_NAME);
			
			server->client_count++;
			return i;
		}
	}

	return -1; // Server full
}

// Remove a client from the server
void remove_client(Server* server, int client_index)
{
	if (!server || client_index < 0 || client_index >= MAX_CLIENTS)
{
		return;
	}

	if (server->clients[client_index].is_active)
{
		printf("Removing client '%s'\n", server->clients[client_index].client_id);
		
		safe_close_socket(&server->clients[client_index].socket_fd);
		server->clients[client_index].is_active = 0;
		server->clients[client_index].inactive_count = 0;
		server->clients[client_index].thread_id = 0;
		memset(server->clients[client_index].client_id, 0, MAX_CLIENT_NAME);
		
		server->client_count--;
	}
}

// Find client by ID
int find_client_by_id(Server* server, const char* client_id)
{
	if (!server || !client_id)
{
		return -1;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
{
		if (server->clients[i].is_active && 
			strcmp(server->clients[i].client_id, client_id) == 0)
{
			return i;
		}
	}

	return -1;
}

// Find client by socket
int find_client_by_socket(Server* server, int socket)
{
	if (!server || socket < 0)
{
		return -1;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
{
		if (server->clients[i].is_active && 
			server->clients[i].socket_fd == socket)
{
			return i;
		}
	}

	return -1;
}

// Handle individual client communication
void* handle_client(void* arg)
{
	ServerClient* client = (ServerClient*)arg;
	if (!client || !g_server)
{
		return NULL;
	}

	Message msg;
	int result;

	printf("Thread started for client '%s'\n", client->client_id);

	while (g_server->is_running && client->is_active)
{
		result = receive_message_protocol(client->socket_fd, &msg);
		
		if (result <= 0)
{
			if (result == 0)
{
				printf("Client '%s' disconnected cleanly\n", client->client_id);
			} else {
				printf("Error receiving message from client '%s'\n", client->client_id);
			}
			break;
		}

		// Update last seen time
		client->last_seen = time(NULL);
		client->inactive_count = 0;

		// Log received message
		log_message(&msg, "RECV");

		// Handle different message types
		switch (msg.type)
{
			case MSG_LIST:
				handle_list_request(g_server, client->socket_fd);
				break;
			case MSG_2ALL:
				handle_2all_request(g_server, &msg, client->socket_fd);
				break;
			case MSG_2ONE:
				handle_2one_request(g_server, &msg, client->socket_fd);
				break;
			case MSG_STOP:
				handle_stop_request(g_server, client->socket_fd);
				goto cleanup;
			case MSG_ALIVE_RESPONSE:
				handle_alive_response(g_server, client->socket_fd);
				break;
			default:
				printf("Unknown message type %d from client '%s'\n", 
					   msg.type, client->client_id);
				break;
		}
	}

cleanup:
	// Remove client from server
	pthread_mutex_lock(&g_server->clients_mutex);
	int client_index = find_client_by_socket(g_server, client->socket_fd);
	if (client_index != -1)
{
		remove_client(g_server, client_index);
	}
	pthread_mutex_unlock(&g_server->clients_mutex);

	printf("Thread ended for client '%s'\n", client->client_id);
	return NULL;
}

// Handle LIST request
void handle_list_request(Server* server, int client_socket)
{
	if (!server)
{
		return;
	}

	send_client_list(server, client_socket);
}

// Handle 2ALL request (broadcast)
void handle_2all_request(Server* server, const Message* msg, int sender_socket)
{
	if (!server || !msg)
{
		return;
	}

	Message broadcast_msg = create_message(MSG_BROADCAST, msg->sender_id, "", msg->content);
	broadcast_message(server, &broadcast_msg, sender_socket);
}

// Handle 2ONE request (private message)
void handle_2one_request(Server* server, const Message* msg, int sender_socket)
{
	if (!server || !msg)
	{
		return;
	}

	Message private_msg = create_message(MSG_PRIVATE, msg->sender_id, 
									   msg->recipient_id, msg->content);
	send_private_message(server, &private_msg, msg->recipient_id);
}

// Handle STOP request
void handle_stop_request(Server* server, int client_socket)
{
	if (!server)
	{
		return;
	}

	Message response = create_message(MSG_STOP, "SERVER", "", "Goodbye!");
	send_message_protocol(client_socket, &response);
}

// Handle ALIVE response
void handle_alive_response(Server* server, int client_socket)
{
	if (!server)
	{
		return;
	}

	pthread_mutex_lock(&server->clients_mutex);
	int client_index = find_client_by_socket(server, client_socket);
	if (client_index != -1)
{
		server->clients[client_index].last_seen = time(NULL);
		server->clients[client_index].inactive_count = 0;
	}
	pthread_mutex_unlock(&server->clients_mutex);
}

// Send client list to requesting client
void send_client_list(Server* server, int client_socket)
{
	if (!server)
	{
		return;
	}

	char client_list[MAX_MESSAGE_SIZE] = {0};
	safe_strcpy(client_list, "Connected clients: ", sizeof(client_list));

	pthread_mutex_lock(&server->clients_mutex);
	
	int first = 1;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (server->clients[i].is_active)
		{
			if (!first)
			{
				safe_strcat(client_list, ", ", sizeof(client_list));
			}
			safe_strcat(client_list, server->clients[i].client_id, sizeof(client_list));
			first = 0;
		}
	}
	
	pthread_mutex_unlock(&server->clients_mutex);

	if (first)
	{
		safe_strcpy(client_list, "No clients connected", sizeof(client_list));
	}

	Message list_msg = create_message(MSG_CLIENT_LIST, "SERVER", "", client_list);
	send_message_protocol(client_socket, &list_msg);
}

// Broadcast message to all clients except sender
void broadcast_message(Server* server, const Message* msg, int sender_socket)
{
	if (!server || !msg)
	{
		return;
	}

	pthread_mutex_lock(&server->clients_mutex);
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (server->clients[i].is_active && 
			server->clients[i].socket_fd != sender_socket)
		{
			
			int result = send_message_protocol(server->clients[i].socket_fd, msg);
			if (result <= 0)
			{
				printf("Failed to send broadcast to client '%s'\n", 
					   server->clients[i].client_id);
			} else {
				log_message(msg, "SENT");
			}
		}
	}
	
	pthread_mutex_unlock(&server->clients_mutex);
}

// Send private message to specific client
void send_private_message(Server* server, const Message* msg, const char* recipient_id)
{
	if (!server || !msg || !recipient_id)
	{
		return;
	}

	pthread_mutex_lock(&server->clients_mutex);
	
	int recipient_index = find_client_by_id(server, recipient_id);
	if (recipient_index != -1)
	{
		int result = send_message_protocol(server->clients[recipient_index].socket_fd, msg);
		if (result <= 0)
		{
			printf("Failed to send private message to client '%s'\n", recipient_id);
		} else {
			log_message(msg, "SENT");
		}
	} else {
		printf("Recipient '%s' not found for private message\n", recipient_id);
	}
	
	pthread_mutex_unlock(&server->clients_mutex);
}

// Monitor client keepalive status
void* keepalive_monitor(void* arg)
{
	Server* server = (Server*)arg;
	if (!server)
	{
		return NULL;
	}

	printf("Keepalive monitor thread started\n");

	while (server->is_running)
	{
		sleep(KEEPALIVE_INTERVAL);

		if (!server->is_running)
		{
			break;
		}

		time_t now = time(NULL);
		
		pthread_mutex_lock(&server->clients_mutex);
		
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (!server->clients[i].is_active)
			{
				continue;
			}

			// Check if client has been inactive too long
			if (now - server->clients[i].last_seen > KEEPALIVE_INTERVAL)
			{
				server->clients[i].inactive_count++;
				
				if (server->clients[i].inactive_count >= MAX_INACTIVE_COUNT)
				{
					printf("Client '%s' timeout - removing\n", 
						   server->clients[i].client_id);
					remove_client(server, i);
					continue;
				}

				// Send keepalive message
				Message alive_msg = create_message(MSG_ALIVE, "SERVER", 
												 server->clients[i].client_id, "");
				int result = send_message_protocol(server->clients[i].socket_fd, &alive_msg);
				
				if (result <= 0)
				{
					printf("Failed to send keepalive to client '%s' - removing\n", 
						   server->clients[i].client_id);
					remove_client(server, i);
				}
			}
		}
		
		pthread_mutex_unlock(&server->clients_mutex);
	}

	printf("Keepalive monitor thread ended\n");
	return NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return 1;
	}

	int port = atoi(argv[1]);
	if (port < PORT_MIN || port > PORT_MAX)
	{
		fprintf(stderr, "Invalid port number. Must be between %d and %d\n", 
				PORT_MIN, PORT_MAX);
		return 1;
	}

	Server server;
	if (server_init(&server, port) == -1)
	{
		fprintf(stderr, "Failed to initialize server\n");
		return 1;
	}

	printf("Starting chat server on port %d\n", port);
	server_start(&server);

	server_cleanup(&server);
	printf("Server terminated\n");
	return 0;
}
