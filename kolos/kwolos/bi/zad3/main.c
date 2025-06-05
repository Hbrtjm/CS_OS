#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define MSG_SIZE 20

mqd_t open_msg_queue(const char* name) {
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(name, O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    return mq;
}

int main(void) {
    char buf[MSG_SIZE];
    mqd_t queue = open_msg_queue("/msgs");

    puts("[Parent] Trying to receive a msg...");
    if (mq_receive(queue, buf, sizeof(buf), NULL) == -1) {
        if (errno == EAGAIN) {
            puts("[Parent] Ok, no messages yet");
        } else {
            perror("[Parent]");
            exit(-1);
        }
    }

    pid_t pid = fork();
    if (pid != 0) {
        wait(NULL);
        puts("[Parent] Trying to receive a msg once more...");
        int bytes = mq_receive(queue, buf, sizeof(buf), NULL);
        printf("[Parent] Received %d bytes: %s\n", bytes, buf);
    } else {
        const char* data = "Konstantynopolitanczykowianeczka";
        if (mq_send(queue, data, strlen(data) + 1, 0) == -1) {
            if (errno == EMSGSIZE) {
                printf("[Child] '%s' too long, let's try something else\n", data);
            } else {
                perror("[Child]");
                exit(-1);
            }
        }
        data = "Ala ma kota";
        mq_send(queue, data, strlen(data) + 1, 0);
    }

    mq_close(queue);
    mq_unlink("/msgs");

    return 0;
}
