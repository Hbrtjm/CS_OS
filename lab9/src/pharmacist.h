#ifndef PHARMACIST_H
#define PHARMACIST_H

#include "common.h"

struct Pharmacist {
	int id;
	pthread_t thread;
	Clinic* clinic;
};

void pharmacist_init(PharmacistTypeDef* pharmacist, int id, Clinic* clinic);
void* pharmacist_thread_routine(void* arg);
void pharmacist_print_action(const PharmacistTypeDef* pharmacist, const char* message);
void pharmacist_go_to_hospital(PharmacistTypeDef* pharmacist);
void pharmacist_wait_for_medicine_cabinet(PharmacistTypeDef* pharmacist);
void pharmacist_wake_doctor(PharmacistTypeDef* pharmacist);
void pharmacist_medicine_delivery(PharmacistTypeDef* pharmacist);
void pharmacist_finish_work(PharmacistTypeDef* pharmacist);

#endif
