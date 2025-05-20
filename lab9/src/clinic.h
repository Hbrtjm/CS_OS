#ifndef CLINIC_H
#define CLINIC_H

#include "doctor.h"
#include "queue.h"
#include "common.h"

struct Clinic {
	DoctorTypeDef *doctor;
	PatientQueueTypeDef *patient_queue;
		
	int all_patients_done;
	int waiting_pharmacists;
	int patients_treated;
	int medicine_count;
	pthread_mutex_t medicine_mutex;
	MedicineStatus medicine_status;
};

void clinic_init(Clinic* clinic);
void clinic_destroy(Clinic* clinic);
void clinic_add_patient(Clinic* clinic, PatientTypeDef* patient);
void clinic_add_medicine(Clinic* clinic, int amount);
void clinic_use_medicine(Clinic* clinic, int amount);
int clinic_has_enough_medicine(const Clinic* clinic);
void clinic_signal_patients_done(Clinic* clinic);

#endif
