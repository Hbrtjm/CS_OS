#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define MAX_INACTIVE_COUNT 3
#define KEEPALIVE_INTERVAL 30  // seconds

typedef struct {
    int socket_fd;
    char client_id[MAX_CLIENT_NAME];
    struct sockaddr_in addr;
    time_t last_seen;
    int inactive_count;
    int is_active;
    pthread_t thread_id;
} ServerClient;

typedef struct {
    int server_socket;
    int port;
    ServerClient clients[MAX_CLIENTS];
    int client_count;
    int is_running;
    pthread_mutex_t clients_mutex;
    pthread_t keepalive_thread;
} Server;

int server_init(Server* server, int port);
void server_start(Server* server);
void server_stop(Server* server);
void server_cleanup(Server* server);

int add_client(Server* server, int client_socket, struct sockaddr_in client_addr, const char* client_id);
void remove_client(Server* server, int client_index);
int find_client_by_id(Server* server, const char* client_id);
int find_client_by_socket(Server* server, int socket);

void* handle_client(void* arg);
void handle_list_request(Server* server, int client_socket);
void handle_2all_request(Server* server, const Message* msg, int sender_socket);
void handle_2one_request(Server* server, const Message* msg, int sender_socket);
void handle_stop_request(Server* server, int client_socket);
void handle_alive_response(Server* server, int client_socket);

void* keepalive_monitor(void* arg);
void send_client_list(Server* server, int client_socket);
void broadcast_message(Server* server, const Message* msg, int sender_socket);
void send_private_message(Server* server, const Message* msg, const char* recipient_id);

#endif
