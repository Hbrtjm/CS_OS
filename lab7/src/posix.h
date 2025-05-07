#ifndef POSIX_H
#define POSIX_H

#pragma once

#define _XOPEN_SOURCE 500

#define MSG_SIZE 11
#define SEM_NAME "/sem_jobs"
#define SHARED_NAME "/shm_jobs"
#define MAX_MESSAGES 20
#define DEBUG 1
#define PRINT_TIMEOUT 1
#define FINISHED_MESSAGE "Done"

typedef int shm_t;

typedef struct {
    char jobs[MAX_MESSAGES][MSG_SIZE];
    volatile int in;
    volatileint out;
} shared_memory_t;

#endif

