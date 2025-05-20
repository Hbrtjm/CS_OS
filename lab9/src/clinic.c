#include "doctor.h"
#include "pharmacist.h"
#include "patient.h"
#include "queue.h"
#include "clinic.h"

void clinic_init(Clinic* clinic) {
	if (!clinic) return;
	
	clinic->all_patients_done = 0;
	clinic->waiting_pharmacists = 0;
	clinic->patients_treated = 0;
	clinic->medicine_count = MEDICINE_CABINET_CAPACITY; 
	
	clinic->patient_queue = (PatientQueueTypeDef*)malloc(sizeof(PatientQueueTypeDef));
	clinic->doctor = (DoctorTypeDef*)malloc(sizeof(DoctorTypeDef));
	
	pthread_mutexattr_t attr = get_attr();
	pthread_mutex_init(&clinic->medicine_mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	queue_init(clinic->patient_queue);
	doctor_init(clinic->doctor, clinic);
}

void clinic_destroy(Clinic* clinic) {
	if (!clinic) return;
	
	pthread_mutex_destroy(&clinic->medicine_mutex);
	queue_destroy(clinic->patient_queue);
	doctor_destroy(clinic->doctor);
}

void clinic_add_patient(Clinic* clinic, PatientTypeDef* patient) {
	if (!clinic || !patient) return;
	
	pthread_mutex_lock(&clinic->patient_queue->mutex);
	queue_add(clinic->patient_queue, patient);
	pthread_mutex_unlock(&clinic->patient_queue->mutex);
}

void clinic_add_medicine(Clinic* clinic, int amount) {
	if (!clinic) return;
	
	pthread_mutex_lock(&clinic->medicine_mutex);
	clinic->medicine_count += amount;
	if (clinic->medicine_count > MEDICINE_CABINET_CAPACITY) {
		clinic->medicine_count = MEDICINE_CABINET_CAPACITY;
	}
	pthread_mutex_unlock(&clinic->medicine_mutex);
}

void clinic_use_medicine(Clinic* clinic, int amount) {
	if (!clinic) return;
	
	pthread_mutex_lock(&clinic->medicine_mutex);
	clinic->medicine_count -= amount;
	if (clinic->medicine_count < 0) {
		clinic->medicine_count = 0;
	}
	pthread_mutex_unlock(&clinic->medicine_mutex);
}

int clinic_has_enough_medicine(const Clinic* clinic) {
	if (!clinic) return 0;
	
	int result = 0;
	pthread_mutex_lock(&((Clinic*)clinic)->medicine_mutex);
	result = clinic->medicine_count >= MIN_MEDICINE_REQUIRED;
	pthread_mutex_unlock(&((Clinic*)clinic)->medicine_mutex);
	
	return result;
}

void clinic_signal_patients_done(Clinic* clinic) {
    if (!clinic) return;
    
    pthread_mutex_lock(&clinic->medicine_mutex);
    clinic->all_patients_done = 1;
    pthread_mutex_unlock(&clinic->medicine_mutex);
    
    pthread_mutex_lock(&clinic->doctor->mutex);
    pthread_cond_signal(&clinic->doctor->condition);
    pthread_mutex_unlock(&clinic->doctor->mutex);
    
    pthread_mutex_lock(&clinic->medicine_mutex);
    clinic->medicine_status = RECEIVED; 
    pthread_cond_broadcast(&clinic->doctor->condition);
    pthread_mutex_unlock(&clinic->medicine_mutex);
}
