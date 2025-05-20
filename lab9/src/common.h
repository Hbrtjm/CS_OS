
#ifndef COMMON_H
#define COMMON_H

#pragma once

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Common defines
#define MAX_MSG_SIZE 128
#define DEFAULT_PATIENTS 10
#define DEFAULT_PHARMA 3
#define MEDICINE_CABINET_CAPACITY 6
#define MIN_MEDICINE_REQUIRED 3
#define CLINIC_QUEUE_SIZE 3
#define MAX_VISIT_RETRIES 16

typedef enum {
	ARRIVING,
	WALKING,
	QUEUE,
	TREATMENT,
	GONE
} PatientStatus;

typedef enum {
	SLEEPING,
	TREATING_PATIENTS,
	ACCEPTING_PHARMACY
} DoctorStatus;

typedef enum {
	IN_DELIVERY,
	RECEIVED
} MedicineStatus;


typedef struct Patient PatientTypeDef;
typedef struct PatientQueue PatientQueueTypeDef;
typedef struct Doctor DoctorTypeDef;
typedef struct Pharmacist PharmacistTypeDef;
typedef struct Clinic Clinic;

pthread_mutexattr_t get_attr();
void print_timestamp_message(const char* actor, int id, const char* message);
unsigned int get_random_range(unsigned int min, unsigned int max);

#endif
