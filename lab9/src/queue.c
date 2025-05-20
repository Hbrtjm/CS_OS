#include "queue.h"
#include "common.h"
#include "patient.h"
#include <pthread.h>

#define QUEUE_SIZE CLINIC_QUEUE_SIZE + 1

void queue_init(PatientQueueTypeDef* queue) {
	if (!queue) return;

	queue->head = 0;
	queue->tail = 0;
	pthread_mutexattr_t attr = get_attr();
	pthread_mutex_init(&queue->mutex, &attr);
	pthread_mutexattr_destroy(&attr);

	for (int i = 0; i < QUEUE_SIZE; i++) {
		queue->queue_array[i] = NULL;
	}
}

void queue_destroy(PatientQueueTypeDef* queue) {
	if (!queue) return;
	pthread_mutex_destroy(&queue->mutex);
}

int queue_is_full(PatientQueueTypeDef* queue) {
	return ((queue->tail + 1) % QUEUE_SIZE) == queue->head;
}

int queue_is_empty(PatientQueueTypeDef* queue) {
	return queue->head == queue->tail;
}

int queue_add(PatientQueueTypeDef* queue, PatientTypeDef* patient) {
	if (!queue || !patient) return -1;
	pthread_mutex_lock(&queue->mutex);
	if (queue_is_full(queue)) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}

	queue->queue_array[queue->tail] = patient;
	queue->tail = (queue->tail + 1) % QUEUE_SIZE;
	pthread_mutex_unlock(&queue->mutex);
	return 0;
}

PatientTypeDef* queue_pop(PatientQueueTypeDef* queue) {
	if (!queue) return NULL;

	pthread_mutex_lock(&queue->mutex);
	if (queue_is_empty(queue)) {
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}

	PatientTypeDef* patient = queue->queue_array[queue->head];
	queue->queue_array[queue->head] = NULL;
	queue->head = (queue->head + 1) % QUEUE_SIZE;
	pthread_mutex_unlock(&queue->mutex);

	return patient;
}

int queue_count(PatientQueueTypeDef* queue) {
	if (!queue) return 0;
	pthread_mutex_lock(&queue->mutex);
	int diff = (queue->tail - queue->head + QUEUE_SIZE) % QUEUE_SIZE;
		pthread_mutex_unlock(&queue->mutex);
	return diff;
}

void clear_queue(PatientQueueTypeDef* queue) {
	if (!queue) return;

	pthread_mutex_lock(&queue->mutex);
	queue->head = queue->tail = 0;
	for (int i = 0; i < QUEUE_SIZE; i++) {
		queue->queue_array[i] = NULL;
	}
	pthread_mutex_unlock(&queue->mutex);
}

