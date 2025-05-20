#include "patient.h"
#include "queue.h"
#include "clinic.h"
#include "common.h"

void patient_init(PatientTypeDef* patient, int id, Clinic* clinic) {
	if (!patient || !clinic) return;
		
	patient->id = id;
	patient->status = ARRIVING;
	patient->wait_time = 0;
	patient->visit_attempts = 0;
	patient->clinic = clinic;
	
	// printf("Patient of ID %d created\n", id);
	// fflush(stdout);
}

void patient_print_action(const PatientTypeDef* patient, const char* message) {
	if (!patient || !message) return;
	
	print_timestamp_message("Pacjent", patient->id, message);
}

void patient_go_to_hospital(PatientTypeDef* patient) {
	if (!patient) return;
	
	int arrival_time = get_random_range(2, 5);
	char msg[MAX_MSG_SIZE];

	sprintf(msg, "Ide do szpitala, bede za %d s", arrival_time);
	patient_print_action(patient, msg);
	
	patient->wait_time = arrival_time;
	patient->status = ARRIVING;
	
	sleep(arrival_time);
}

void patient_waiting_walk(PatientTypeDef* patient) {
	if (!patient) return;
	
	patient->visit_attempts++;
	
	int wait_time = get_random_range(1, 5);
	char msg[MAX_MSG_SIZE];
	
	sprintf(msg, "Za duzo pacjentow, wroce za %d s", wait_time);
	patient_print_action(patient, msg);
	
	patient->wait_time = wait_time;
	patient->status = WALKING;
	
	sleep(wait_time);
}

void patient_wake_doctor(PatientTypeDef* patient) {
	if (!patient) return;
	
	patient_print_action(patient, "Budze lekarza");
}

void patient_leave_without_visit(PatientTypeDef* patient) {
	if (!patient) return;
	
	patient_print_action(patient, "Poddaje sie");
	patient->status = GONE;
}

void patient_end_visit(PatientTypeDef* patient) {
	if (!patient) return;
	
	patient_print_action(patient, "Koncze wizyte");
	patient->status = GONE;
}

void* patient_thread_routine(void* arg) {
	PatientTypeDef* patient = (PatientTypeDef*)arg;
	PatientQueueTypeDef* queue;
	Clinic* clinic;
	char msg[MAX_MSG_SIZE];
	
	if (!patient || !patient->clinic) {
		return NULL;
	}
	
	clinic = patient->clinic;
	queue = clinic->patient_queue;
	// printf("Starting patient thread of ID %d", patient->id);
	// fflush(stdout);
	// Go to hospital
	patient_go_to_hospital(patient);
	int patient_count;
	int added;
	while (patient->status != GONE) {
		// Try to enter waiting room 
		added = queue_add(queue, patient);
	
	if (added == -1) {
			
			patient_waiting_walk(patient);
			
			if (patient->visit_attempts > MAX_VISIT_RETRIES) {
				patient_leave_without_visit(patient);
				break;
			}
			
			continue;
		}

		
	patient->status = QUEUE;
		patient_count = queue_count(queue);

	sprintf(msg, "Czekam na doktora z %d pacjentami w kolejce", patient_count - 1);
		patient_print_action(patient, msg);
		
		if (patient_count == CLINIC_QUEUE_SIZE && clinic->doctor->status == SLEEPING) {
			
			pthread_mutex_lock(&clinic->doctor->mutex);
			patient_wake_doctor(patient);
			pthread_cond_signal(&clinic->doctor->condition);
			pthread_mutex_unlock(&clinic->doctor->mutex);
		}	
		
	while (patient->status == QUEUE) {
			sleep(1);
			
		if (patient->status == TREATMENT) {
			while (patient->status == TREATMENT);			
			patient_end_visit(patient);
			break;
			}
		}
		
		if (patient->status == GONE) {
			break;
		}
	}
	
	patient->status = GONE;
	
	// printf("Patient %d thread ending\n", patient->id);
	// fflush(stdout);
	
	return NULL;
}
