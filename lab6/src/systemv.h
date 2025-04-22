#ifndef SYSTEMV_H
#define SYSTEMV_H

#define MSG_SIZE        1024
#define MAX_CLIENTS       10

#define SERVER_KEY_PATH   "."
#define SERVER_PROJ_ID    65
#define CLIENT_PROJ_ID     1

struct msg {
    long msg_type;
    char message[MSG_SIZE];
};

struct client {
    int  active;
    int  qid;
};

#endif

