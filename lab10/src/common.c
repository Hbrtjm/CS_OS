#include "common.h"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <ctype.h>

void format_timestamp(time_t time, char* buffer, size_t buffer_size) 
{
	if (!buffer || buffer_size == 0) 
	{
		return;
	}

	struct tm* tm_info = localtime(&time);
	if (!tm_info) {
		strncpy(buffer, "Invalid time", buffer_size - 1);
		buffer[buffer_size - 1] = '\0';
		return;
	}

	// Format: HH:MM:SS
	strftime(buffer, buffer_size, "%H:%M:%S", tm_info);
}

int send_message_protocol(int socketfd, const Message* message) 
{
	if (socketfd < 0 || !message)
	{
		fprintf(stderr, "Invalid socket or message pointer is NULL\n");
		return -1;
	}

	char buffer[sizeof(Message)];
	ssize_t bytes_sent = 0;
	ssize_t total_bytes = sizeof(Message);

	memcpy(buffer, message, sizeof(Message));

	while (bytes_sent < total_bytes)
	{
		ssize_t result = send(socketfd, buffer + bytes_sent, total_bytes - bytes_sent, MSG_NOSIGNAL);
		if (result == -1)
		{
			if (errno == EINTR)
			{
				// Interrupted by signal
				continue;
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Socket would block, wait a bit and try again
				sleep(1);
				continue;
			}
			perror("Send message protocol error");
			return -1;
		}
		if (result == 0)
		{
			// Connection closed
			fprintf(stderr, "Connection closed during send\n");
			return 0;
		}
		bytes_sent += result;
	}
	return bytes_sent;
}

int receive_message_protocol(int socketfd, Message* message)
{
	if (socketfd < 0 || !message)
	{
		fprintf(stderr, "Invalid socket or message pointer is NULL\n");
		return -1;
	}

	char buffer[sizeof(Message)];
	memset(message, 0, sizeof(Message));
	ssize_t read_bytes = 0;
	ssize_t result = 0;
	ssize_t total_bytes = sizeof(Message);
	
	while (read_bytes < total_bytes) 
	{
		result = recv(socketfd, buffer + read_bytes, total_bytes - read_bytes, 0);
		
		if (result < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				sleep(1);
				continue;
			}
			perror("Error while reading the buffer");
			return -1;
		}
		if (result == 0)
		{
			if (read_bytes == 0)
			{
				// Clean connection close
				return 0;
			}
			else
			{
				fprintf(stderr, "Message received partially, connection was broken\n");
				return -1;
			}
		}
		read_bytes += result;
	}

	// Copy buffer bytes into the message
	memcpy(message, buffer, sizeof(Message));

	// Validate message
	if (!validate_message(message)) {
		fprintf(stderr, "Received invalid message\n");
		return -1;
	}

	return read_bytes;
}

int validate_client_id(const char* id)
{
	if (!id) 
	{
		return 0;
	}

	size_t len = strlen(id);

	if (len == 0 || len >= MAX_CLIENT_NAME) 
	{
		return 0;
	}

	// Check first character is not a digit
	if (isdigit(id[0])) {
		return 0;
	}

	// Check all characters are alphanumeric or underscore
	for (size_t i = 0; i < len; i++)
	{
		if (!isalnum(id[i]) && id[i] != '_') 
		{
			return 0;
		}
	}

	return 1;
}

int validate_message(const Message* message) 
{
	if (!message)
	{
		return 0;
	}
	
	// Check message type is valid
	if (message->type < MSG_LIST || message->type > MSG_PRIVATE)
	{
		return 0;
	}
	
	// Check content length is valid
	if (message->content_length < 0 || message->content_length >= MAX_MESSAGE_SIZE)
	{
		return 0;
	}
	
	// Check that string fields are null-terminated
	if (message->sender_id[MAX_CLIENT_NAME - 1] != '\0')
	{
		return 0;
	}
	
	if (message->recipient_id[MAX_CLIENT_NAME - 1] != '\0')
	{
		return 0;
	}

	if (message->content[MAX_MESSAGE_SIZE - 1] != '\0')
	{
		return 0;
	}
	
	// Check timestamp is reasonable (not too old)
	time_t now = time(NULL);
	if (message->timestamp < now - 3600) {  // 1 hour old
		return 0;
	}
	
	// Check timestamp is not in the future (with some tolerance for clock skew)
	if (message->timestamp > now + 300) {   // 5 minutes in the future
		return 0;
	}
	
	return 1;
}

void format_full_timestamp(time_t time, char* buffer, size_t buffer_size) 
{
	if (!buffer || buffer_size == 0) 
	{
		return;
	}

	struct tm* tm_info = localtime(&time);
	if (!tm_info) 
	{
		strncpy(buffer, "Invalid time", buffer_size - 1);
		buffer[buffer_size - 1] = '\0';
		return;
	}

	strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

Message create_message(MessageType type, const char* sender, const char* recipient, const char* content) 
{
	Message msg = {0};

	msg.type = type;
	msg.timestamp = time(NULL);

	if (sender) 
	{
		strncpy(msg.sender_id, sender, MAX_CLIENT_NAME - 1);
		msg.sender_id[MAX_CLIENT_NAME - 1] = '\0';
	}

	if (recipient) 
	{
		strncpy(msg.recipient_id, recipient, MAX_CLIENT_NAME - 1);
		msg.recipient_id[MAX_CLIENT_NAME - 1] = '\0';
	}

	if (content) 
	{
		strncpy(msg.content, content, MAX_MESSAGE_SIZE - 1);
		msg.content[MAX_MESSAGE_SIZE - 1] = '\0';
		msg.content_length = strlen(msg.content);
	}

	return msg;
}

void safe_strcpy(char* dest, const char* src, size_t dest_size) {
	if (!dest || !src || dest_size == 0) 
	{
		return;
	}

	strncpy(dest, src, dest_size - 1);
	dest[dest_size - 1] = '\0';
}

void safe_strcat(char* dest, const char* src, size_t dest_size) {
	if (!dest || !src || dest_size == 0) 
	{
		return;
	}

	size_t dest_len = strlen(dest);
	if (dest_len >= dest_size - 1) 
	{
		return;
	}

	strncat(dest, src, dest_size - dest_len - 1);
}

int parse_address(const char* addr_str, char* ip, size_t ip_size, int* port)
{
	if (!addr_str || !ip || !port) 
	{
		return -1;
	}

	char temp[256];
	safe_strcpy(temp, addr_str, sizeof(temp));

	char* colon = strrchr(temp, ':');
	if (!colon) 
	{
		return -1;
	}

	*colon = '\0';
	char* port_str = colon + 1;

	if (strlen(temp) == 0 || strlen(temp) >= ip_size) 
	{
		return -1;
	}

	struct sockaddr_in sa;
	if (inet_pton(AF_INET, temp, &(sa.sin_addr)) != 1) 
	{
		return -1;
	}

	safe_strcpy(ip, temp, ip_size);

	char* endptr;
	long port_long = strtol(port_str, &endptr, 10);

	if (*endptr != '\0' || port_long < PORT_MIN || port_long > PORT_MAX) 
	{
		return -1;
	}

	*port = (int)port_long;
	return 0;
}

void get_current_timestamp(char* buffer, size_t buffer_size)
{
	time_t now = time(NULL);
	format_timestamp(now, buffer, buffer_size);
}

char* trim_whitespace(char* str) 
{
	if (!str) 
	{
		return NULL;
	}

	// Trim leading whitespace
	// while (isspace((unsigned char)*str)) {
	//	 str = str + 1;
	// }

	// if (*str == '\0') {
	//	 return str;
	// }

	// Trim trailing whitespace
	// char* end = str + strlen(str) - 1;
	// while (end > str && isspace((unsigned char)*end)) {
	//	 end--;
	// }

	// end[1] = '\0';
	return str;
}

const char* message_type_to_string(MessageType type) 
{
	switch (type) {
		case MSG_LIST:
			return "LIST";
		case MSG_2ALL:
			return "2ALL";
		case MSG_2ONE:
			return "2ONE";
		case MSG_STOP:
			return "STOP";
		case MSG_ALIVE:
			return "ALIVE";
		case MSG_ALIVE_RESPONSE:
			return "ALIVE_RESPONSE";
		case MSG_ERROR:
			return "ERROR";
		case MSG_CLIENT_LIST:
			return "CLIENT_LIST";
		case MSG_BROADCAST:
			return "BROADCAST";
		case MSG_PRIVATE:
			return "PRIVATE";
		default:
			return "UNKNOWN";
	}
}

void log_message(const Message* msg, const char* direction) {
	if (!msg || !direction) {
		return;
	}

	char timestamp[64];
	format_full_timestamp(msg->timestamp, timestamp, sizeof(timestamp));

	printf("[%s] %s: %s -> %s | Type: %s | Content: %.50s%s\n",
		   timestamp,
		   direction,
		   msg->sender_id,
		   strlen(msg->recipient_id) > 0 ? msg->recipient_id : "ALL",
		   message_type_to_string(msg->type),
		   msg->content,
		   strlen(msg->content) > 50 ? "..." : "");
}

int set_socket_nonblocking(int socket_fd) {
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags == -1) 
	{
		perror("fcntl F_GETFL");
		return -1;
	}

	if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) 
	{
		perror("fcntl F_SETFL O_NONBLOCK");
		return -1;
	}

	return 0;
}

int set_socket_blocking(int socket_fd) 
{
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags == -1) 
	{
		perror("fcntl F_GETFL");
		return -1;
	}

	if (fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK) == -1) 
	{
		perror("fcntl F_SETFL blocking");
		return -1;
	}

	return 0;
}

int configure_socket(int socket_fd) 
{
	int reuse = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) 
	{
		perror("setsockopt SO_REUSEADDR");
		return -1;
	}
	int keepalive = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == -1) 
	{
		perror("setsockopt SO_KEEPALIVE");
		return -1;
	}
	int nodelay = 1;
	if (setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) == -1) 
	{
		perror("setsockopt TCP_NODELAY");
		return -1;
	}

	return 0;
}

int create_server_socket(int port) {
	int server_socket;
	struct sockaddr_in server_addr;

	// Create socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) 
	{
		perror("Server socket creation failed");
		return -1;
	}

	// Configure socket
	if (configure_socket(server_socket) == -1) 
	{
		close(server_socket);
		return -1;
	}

	// Bind socket
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
	{
		perror("Server socket bind failed");
		close(server_socket);
		return -1;
	}

	// Listen for connections
	if (listen(server_socket, MAX_CLIENTS) == -1) 
	{
		perror("Server socket listen failed");
		close(server_socket);
		return -1;
	}

	return server_socket;
}

void safe_close_socket(int* socket_fd) {
	if (socket_fd && *socket_fd != -1) 
	{
		if (close(*socket_fd) == -1) 
		{
			perror("close socket");
		}
		*socket_fd = -1;
	}
}
