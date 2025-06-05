#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define MAX_CLIENT_NAME 100
#define MAX_MESSAGE_SIZE 1024
#define MAX_CLIENTS 255
#define BUFFER_SIZE 2048
#define PORT_MIN 1024
#define PORT_MAX 65535

typedef enum {
	MSG_LIST,
	MSG_2ALL,
	MSG_2ONE,
	MSG_STOP,
	MSG_ALIVE,
	MSG_ALIVE_RESPONSE,
	MSG_ERROR,
	MSG_CLIENT_LIST,
	MSG_BROADCAST,
	MSG_PRIVATE
} MessageType;

typedef struct {
	MessageType type;
	char sender_id[MAX_CLIENT_NAME];
	char recipient_id[MAX_CLIENT_NAME];
	char content[MAX_MESSAGE_SIZE];
	time_t timestamp;
	int content_length;
} Message;

int send_message_protocol(int socket, const Message* msg, struct sockaddr_in *addr, int addr_len);
int receive_message_protocol(int socket, Message* msg);
int validate_message(const Message* msg);
void format_timestamp(time_t time, char* buffer, size_t buffer_size);
void format_full_timestamp(time_t time, char* buffer, size_t buffer_size);
int validate_client_id(const char* id);
Message create_message(MessageType type, const char* sender, const char* recipient, const char* content);

void safe_strcpy(char* dest, const char* src, size_t dest_size);
void safe_strcat(char* dest, const char* src, size_t dest_size);
char* trim_whitespace(char* str);

int parse_address(const char* addr_str, char* ip, size_t ip_size, int* port);
int set_socket_nonblocking(int socket);
int set_socket_blocking(int socket);
int configure_socket(int socket);
int create_server_socket(int port);
void safe_close_socket(int* socket);

void get_current_timestamp(char* buffer, size_t buffer_size);
const char* message_type_to_string(MessageType type);
void log_message(const Message* msg, const char* direction);

#endif
