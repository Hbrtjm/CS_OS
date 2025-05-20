#include "common.h"


pthread_mutexattr_t get_attr() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	return attr;
}

void print_timestamp_message(const char* actor, int id, const char* message) {
	time_t seconds;
	time(&seconds);
	printf("[%ld] - %s(%d): %s\n", seconds, actor, id, message);
}

unsigned int get_random_range(unsigned int min, unsigned int max) {
	return (rand() % (max - min + 1)) + min;
}
