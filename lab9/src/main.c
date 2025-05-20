#include "clinic.h"
#include "common.h"
#include "patient.h"
#include "pharmacist.h"
#include "queue.h"
#include <pthread.h>


int main(int argc, char *argv[]) {
	int patients_count = DEFAULT_PATIENTS;
	int pharmacists_count = DEFAULT_PHARMA;
	Clinic clinic;
	
	if (argc > 1) {
		patients_count = atoi(argv[1]);
		if (patients_count <= 0) {
			printf("Invalid number of patients, using default: %d\n", DEFAULT_PATIENTS);
			patients_count = DEFAULT_PATIENTS;
		}
	}
	
	if (argc > 2) {
		pharmacists_count = atoi(argv[2]);
		if (pharmacists_count <= 0) {
			printf("Invalid number of pharmacists, using default: %d\n", DEFAULT_PHARMA);
			pharmacists_count = DEFAULT_PHARMA;
		}
	}
	
	srand(time(NULL));
	
	clinic_init(&clinic);
	
	PatientTypeDef* patients = (PatientTypeDef*)malloc(patients_count * sizeof(PatientTypeDef*));
	if (!patients) {
		fprintf(stderr, "Failed to allocate memory for patients\n");
		return 1;
	}
	
	PharmacistTypeDef* pharmacists = (PharmacistTypeDef*)malloc(pharmacists_count * sizeof(PharmacistTypeDef*));
	if (!pharmacists) {
		fprintf(stderr, "Failed to allocate memory for pharmacists\n");
		free(patients);
		return 1;
	}
	
	if (pthread_create(&clinic.doctor->thread, NULL, doctor_thread_routine, clinic.doctor) != 0) {
		fprintf(stderr, "Failed to create doctor thread\n");
		free(patients);
		free(pharmacists);
		return 1;
	}
	for (int i = 0; i < patients_count; i++) {
		patient_init(&patients[i], i + 1, &clinic);
		if (pthread_create(&patients[i].thread, NULL, patient_thread_routine, &patients[i]) != 0) {
			fprintf(stderr, "Failed to create patient thread %d\n", i + 1);
		}
	}
	for (int i = 0; i < pharmacists_count; i++) {
		pharmacist_init(&pharmacists[i], i + 1, &clinic);
		if (pthread_create(&pharmacists[i].thread, NULL, pharmacist_thread_routine, &pharmacists[i]) != 0) {
			fprintf(stderr, "Failed to create pharmacist thread %d\n", i + 1);
		}
	}
	for (int i = 0; i < patients_count; i++) {
		pthread_join(patients[i].thread, NULL);
	}

	printf("All patient threads have completed. Signaling clinic shutdown.\n");
	fflush(stdout);
	clinic_signal_patients_done(&clinic);

	clinic_signal_patients_done(&clinic);
	
	pthread_join(clinic.doctor->thread, NULL);
	
	for (int i = 0; i < pharmacists_count; i++) {
		pthread_cancel(pharmacists[i].thread);
	}
	clinic_destroy(&clinic);
	free(patients);
	free(pharmacists);
	
	printf("Simulation completed. Treated %d patients.\n", clinic.patients_treated);
	
	return 0;
}
