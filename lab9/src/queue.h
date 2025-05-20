#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

struct PatientQueue {
    int head;
    int tail;
    PatientTypeDef* queue_array[CLINIC_QUEUE_SIZE+1];
    pthread_mutex_t mutex;
};

void queue_init(PatientQueueTypeDef* queue);
void queue_destroy(PatientQueueTypeDef* queue);
int queue_add(PatientQueueTypeDef* queue, PatientTypeDef* patient);
void clear_queue(PatientQueueTypeDef *queue);
PatientTypeDef* queue_pop(PatientQueueTypeDef* queue);
int queue_count(PatientQueueTypeDef* queue);
int queue_is_full(PatientQueueTypeDef* queue);
int queue_is_empty(PatientQueueTypeDef* queue);

#endif
