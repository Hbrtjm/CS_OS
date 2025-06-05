#include "checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 10
#define VALUE_COUNT 20

int buffer[BUFFER_SIZE];
int count = 0;

// Definicja synchronizacji
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    check_mutex_producer(&mutex);
    check_cond_producer(&producer_cond);

    for (int i = 0; i < VALUE_COUNT; ++i)
    {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE)
        {
            pthread_cond_wait(&producer_cond, &mutex);
        }

        buffer[count++] = i;

        pthread_cond_signal(&consumer_cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *consumer(void *arg)
{
    check_mutex_consumer(&mutex);
    check_cond_consumer(&consumer_cond);

    for (int i = 0; i < VALUE_COUNT; ++i)
    {
        pthread_mutex_lock(&mutex);

        while (count == 0)
        {
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        int value = buffer[--count];
        process_value(value);

        pthread_cond_signal(&producer_cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    check_wait();
    check_results();

    return 0;
}
