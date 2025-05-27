#include "doctor.h"
#include "patient.h"
#include "queue.h"
#include "common.h"
#include "clinic.h"


void doctor_init(DoctorTypeDef* doctor, Clinic* clinic) {
	if (!doctor || !clinic) return;

	doctor->status = SLEEPING;
	doctor->wait_time = 0;
	doctor->clinic = clinic;
	pthread_mutexattr_t attr = get_attr();
	pthread_mutex_init(&doctor->mutex, &attr);
	pthread_cond_init(&doctor->condition, NULL);
	pthread_mutexattr_destroy(&attr);
}

void doctor_destroy(DoctorTypeDef* doctor) {
	if (!doctor) return;
	
	pthread_mutex_destroy(&doctor->mutex);
	pthread_cond_destroy(&doctor->condition);
	free(doctor);
}

void doctor_print_action(const char* message) {
	if (!message) return;
	
	time_t seconds;
	time(&seconds);
	printf("===========================\n[%ld] - Lekarz: %s\n===========================\n", seconds, message);
	fflush(stdout);
}

void doctor_sleep(DoctorTypeDef* doctor) {
	if (!doctor) return;
	
	doctor_print_action("Ide spac");
	doctor->status = SLEEPING;
}

void doctor_wake_up(DoctorTypeDef* doctor) {
	if (!doctor) return;
	
	doctor_print_action("Budze sie");
}

void doctor_handle_patient(DoctorTypeDef* doctor, PatientTypeDef* patient) {
	if (!doctor || !patient) return;
	
	char msg[MAX_MSG_SIZE];
	int visit_time = get_random_range(2, 4);
	sprintf(msg, "Lecze pacjenta %d przez %d", patient->id,visit_time);
	doctor_print_action(msg);
	
	patient->status = TREATMENT;
	
	sleep(visit_time);
	
	patient->status = GONE;
	doctor->clinic->patients_treated++;
}

void doctor_accept_medicine(DoctorTypeDef* doctor) {
	if (!doctor || !doctor->clinic) return;
	
	doctor_print_action("Przyjmuje leki");
	
	int delivery_time = get_random_range(1, 3);
	sleep(delivery_time);
	
	pthread_mutex_lock(&doctor->clinic->medicine_mutex);
	doctor->clinic->medicine_count = MEDICINE_CABINET_CAPACITY;
	doctor->clinic->medicine_status = RECEIVED;
	pthread_cond_signal(&doctor->condition);
	pthread_mutex_unlock(&doctor->clinic->medicine_mutex);
	
	doctor_print_action("Apteczka uzupelniona");
}

void* doctor_thread_routine(void* arg) { 
	DoctorTypeDef* doctor = (DoctorTypeDef*)arg;
	Clinic* clinic;
	PatientQueueTypeDef* queue;
	

	if (!doctor || !doctor->clinic) {
		return NULL;
	}
	
	clinic = doctor->clinic;
	queue = clinic->patient_queue;
	
	doctor_print_action("Zaczynam pracę");

	int patient_count = 0;
	int waking_mode = 0;
	int med_count = 0;
	int pharm_waiting = 0;
	while (!clinic->all_patients_done) {
		
	   pthread_mutex_lock(&doctor->mutex);
	 
		waking_mode = 0;
	 
	if (!waking_mode && !clinic->all_patients_done) {
	
		doctor_sleep(doctor);
	
		pthread_cond_wait(&doctor->condition, &doctor->mutex);
		while (!waking_mode && !clinic->all_patients_done) {
			
			pthread_mutex_lock(&clinic->medicine_mutex);
			med_count = clinic->medicine_count;
			pharm_waiting = clinic->waiting_pharmacists;
			pthread_mutex_unlock(&clinic->medicine_mutex);
			
			patient_count = queue_count(queue);
			
			if (patient_count == CLINIC_QUEUE_SIZE && med_count >= MIN_MEDICINE_REQUIRED) {
				waking_mode = 1;
			}
			else if (pharm_waiting > 0 && med_count < MIN_MEDICINE_REQUIRED) {
				waking_mode = 2;
			}
	}
	} 
		pthread_mutex_unlock(&doctor->mutex);
		
	if (waking_mode == 1) {
			doctor_wake_up(doctor);
		 
			PatientTypeDef* patients_to_treat[CLINIC_QUEUE_SIZE] = {NULL};
			int count = 0;
		 
			int i = 0;
			while (!queue_is_empty(queue)) {
				patients_to_treat[i] = queue_pop(queue);
				if (patients_to_treat[i]) {
					count++;
				}
				i++;
			}
		 	
			//clear_queue(queue);
		        pthread_mutex_lock(&queue->mutex);
			if (count > 0) {
				pthread_mutex_lock(&clinic->medicine_mutex);
				clinic->medicine_count -= MIN_MEDICINE_REQUIRED;
				pthread_mutex_unlock(&clinic->medicine_mutex);
			 
				for (int i = 0; i < count; i++) {
			//		printf("Patient %p", (void*)patients_to_treat[i]);
			  // fflush(stdout);	
		if (patients_to_treat[i]) {
						doctor_handle_patient(doctor, patients_to_treat[i]);
					}
				}
			}
			pthread_mutex_unlock(&queue->mutex);
		}
		else if (waking_mode == 2) {
			doctor_wake_up(doctor);
			doctor->status = ACCEPTING_PHARMACY;
			doctor_accept_medicine(doctor);
		}
	 }
	
	doctor_print_action("Wszyscy pacjenci obsluzeni, koncze.");
	fflush(stdout);
	return NULL;
}

