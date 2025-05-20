#ifndef DOCTOR_H
#define DOCTOR_H

#include "common.h"

struct Doctor {
	DoctorStatus status;
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	int wait_time;
	Clinic* clinic;
};

void doctor_init(DoctorTypeDef* doctor, Clinic* clinic);
void doctor_destroy(DoctorTypeDef* doctor);
void* doctor_thread_routine(void* arg);
void doctor_print_action(const char* message);
void doctor_sleep(DoctorTypeDef* doctor);
void doctor_handle_patient(DoctorTypeDef* doctor, PatientTypeDef* patient);
void doctor_accept_medicine(DoctorTypeDef* doctor);
void doctor_wake_up(DoctorTypeDef* doctor);

#endif
