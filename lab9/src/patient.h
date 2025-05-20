#ifndef PATIENT_H
#define PATIENT_H

#include "common.h"

struct Patient {
    int id;
    PatientStatus status;
    int wait_time;
    int visit_attempts;
    pthread_t thread;
    Clinic* clinic;
};

void patient_init(PatientTypeDef* patient, int id, Clinic* clinic);
void* patient_thread_routine(void* arg);
void patient_print_action(const PatientTypeDef* patient, const char* message);
void patient_go_to_hospital(PatientTypeDef* patient);
void patient_waiting_walk(PatientTypeDef* patient);
void patient_wake_doctor(PatientTypeDef* patient);
void patient_leave_without_visit(PatientTypeDef* patient);
void patient_end_visit(PatientTypeDef* patient);

#endif