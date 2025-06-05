#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MaxItems   5   // Number of items each producer will produce / each consumer will consume
#define BufferSize 5   // Size of the circular buffer

sem_t empty;            // counts empty slots in buffer
sem_t full;             // counts filled slots in buffer
int buffer[BufferSize];
int in = 0;
int out = 0;
pthread_mutex_t mutex;

void *producer(void *pno) {
    int id = *((int *) pno);

    for (int i = 0; i < MaxItems; i++) {
        int item = 10 * id + i;                // generate a random item

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        printf("Producer %d: Insert Item %d at index %d\n", id, item, in);
        in = (in + 1) % BufferSize;

        pthread_mutex_unlock(&mutex);
        sleep(1);
        sem_post(&full);
    }

    return NULL;
}

void *consumer(void *cno) {
    int id = *((int *) cno);

    for (int i = 0; i < MaxItems; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from index %d\n", id, item, out);
        out = (out + 1) % BufferSize;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }

    return NULL;
}

int main() {
    pthread_t pro[5], con[5];
    pthread_mutex_init(&mutex, NULL);

    // Initialize semaphores: 'empty' starts at BufferSize (all slots empty), 'full' starts at 0
    sem_init(&empty, 0, BufferSize);
    sem_init(&full,  0, 0);

    int thread_ids[5] = {1, 2, 3, 4, 5};

    // Create 5 producer threads
    for (int i = 0; i < 5; i++) {
        pthread_create(&pro[i], NULL, producer, &thread_ids[i]);
    }
    // Create 5 consumer threads
    for (int i = 0; i < 5; i++) {
        pthread_create(&con[i], NULL, consumer, &thread_ids[i]);
    }

    // Wait for all producers to finish
    for (int i = 0; i < 5; i++) {
        pthread_join(pro[i], NULL);
    }
    // Wait for all consumers to finish
    for (int i = 0; i < 5; i++) {
        pthread_join(con[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
