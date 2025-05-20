#include "pharmacist.h"
#include "doctor.h"
#include "clinic.h"

void pharmacist_init(PharmacistTypeDef* pharmacist, int id, Clinic* clinic) {
    if (!pharmacist || !clinic) return;
    
    pharmacist->id = id;
    pharmacist->clinic = clinic;
}

void pharmacist_print_action(const PharmacistTypeDef* pharmacist, const char* message) {
    if (!pharmacist || !message) return;
    
    print_timestamp_message("Farmaceuta", pharmacist->id, message);
}

void pharmacist_go_to_hospital(PharmacistTypeDef* pharmacist) {
    if (!pharmacist) return;
    
    int arrival_time = get_random_range(5, 15);
    char msg[MAX_MSG_SIZE];
    
    sprintf(msg, "Ide do szpitala, bede za %d s", arrival_time);
    pharmacist_print_action(pharmacist, msg);
    
    sleep(arrival_time);
}

void pharmacist_wait_for_medicine_cabinet(PharmacistTypeDef* pharmacist) {
    if (!pharmacist) return;
    
    pharmacist_print_action(pharmacist, "Czekam az apteczka sie oprozni");
}

void pharmacist_wake_doctor(PharmacistTypeDef* pharmacist) {
    if (!pharmacist) return;
    
    pharmacist_print_action(pharmacist, "Budze doktora");
}

void pharmacist_medicine_delivery(PharmacistTypeDef* pharmacist) {
    if (!pharmacist) return;
    
    pharmacist_print_action(pharmacist, "Dostarczam leki");
}

void pharmacist_finish_work(PharmacistTypeDef* pharmacist) {
    if (!pharmacist) return;
    
    pharmacist_print_action(pharmacist, "Koncze dostawe");
}

void* pharmacist_thread_routine(void* arg) {
    PharmacistTypeDef* pharmacist = (PharmacistTypeDef*)arg;
    Clinic* clinic;
    
    if (!pharmacist || !pharmacist->clinic) {
        return NULL;
    }
    
    clinic = pharmacist->clinic;
    
    while (!clinic->all_patients_done) {
        pharmacist_go_to_hospital(pharmacist);
        
        pthread_mutex_lock(&clinic->medicine_mutex);
        
        if (clinic->medicine_count >= MEDICINE_CABINET_CAPACITY) {
            pharmacist_wait_for_medicine_cabinet(pharmacist);
            
            while (clinic->medicine_count >= MEDICINE_CABINET_CAPACITY && !clinic->all_patients_done) {
                pthread_mutex_unlock(&clinic->medicine_mutex);
                sleep(1);
                pthread_mutex_lock(&clinic->medicine_mutex);
            }
            
            if (clinic->all_patients_done) {
                pthread_mutex_unlock(&clinic->medicine_mutex);
                return NULL;
            }
        }
        
        if (clinic->medicine_count < MIN_MEDICINE_REQUIRED) {
		clinic->waiting_pharmacists++;
		
		pthread_mutex_lock(&clinic->doctor->mutex);
		pthread_cond_signal(&clinic->doctor->condition);
		pthread_mutex_unlock(&clinic->doctor->mutex);
		pharmacist_wake_doctor(pharmacist);
		
		pthread_mutex_unlock(&clinic->medicine_mutex);
		
		pthread_mutex_lock(&clinic->medicine_mutex);
		clinic->medicine_status = IN_DELIVERY;
		while(clinic->medicine_status == IN_DELIVERY) {
		    pthread_cond_wait(&clinic->doctor->condition, &clinic->medicine_mutex);
		}
		pthread_mutex_unlock(&clinic->medicine_mutex);
		
		pthread_mutex_lock(&clinic->medicine_mutex);
		pharmacist_medicine_delivery(pharmacist);
	}
        
        pthread_mutex_unlock(&clinic->medicine_mutex);
    }
    
    return NULL;
}
