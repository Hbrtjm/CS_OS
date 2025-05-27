#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

// Client structure
typedef struct {
    int server_socket;
    char client_id[MAX_CLIENT_NAME];
    char server_ip[16];  // IPv4 address string
    int server_port;
    int is_connected;
    int is_running;
    pthread_t receive_thread;
    pthread_mutex_t send_mutex;
    pthread_mutex_t state_mutex;  // For thread-safe state access
    pthread_cond_t shutdown_cond; // For coordinated shutdown
} Client;

// Client function prototypes
int client_init(Client* client, const char* client_id, const char* server_ip, int server_port);
int client_connect(Client* client);
void client_disconnect(Client* client);
void client_cleanup(Client* client);

// Communication functions
int client_send_list_request(Client* client);
int client_send_2all(Client* client, const char* message);
int client_send_2one(Client* client, const char* recipient_id, const char* message);
int client_send_stop(Client* client);
int client_send_alive_response(Client* client);

// Message handling
void* client_receive_messages(void* arg);
void handle_received_message(Client* client, const Message* msg);
void display_client_list(const Message* msg);
void display_broadcast_message(const Message* msg);
void display_private_message(const Message* msg);

// User interface functions
void client_run_interactive(Client* client);
void print_help();
void handle_user_input(Client* client, const char* input);

// Signal handling
void setup_signal_handlers(Client* client);
void signal_handler(int sig);

#endif