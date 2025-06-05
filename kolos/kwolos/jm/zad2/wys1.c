#include <string.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define ROZM_BLOKU 1024

int main(void) {
    // Stwórz dwa zamknięte semafory o identyfikatorach id_sem0 i id_sem1
    sem_t *id_sem0 = sem_open("nazwa_sem0", O_CREAT, 0666, 0); // writer signals on this
    sem_t *id_sem1 = sem_open("nazwa_sem1", O_CREAT, 0666, 0); // writer waits on this

    if (id_sem0 == SEM_FAILED || id_sem1 == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Stwórz pamięć współdzieloną, określ jej rozmiar na ROZM_BLOKU
    int shm_fd = shm_open("/pamiec_wspolna", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(shm_fd, ROZM_BLOKU) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Dołącz segment pamięci do przestrzeni adresowej procesu
    char *wsk = mmap(NULL, ROZM_BLOKU, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (wsk == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Główna pętla
    do {
        sem_wait(id_sem1);         // czekaj, aż reader poprosi o dane
        printf("Wpisz dane ('!' aby zakończyć): ");
        fgets(wsk, ROZM_BLOKU, stdin);

        // Usuń znak nowej linii jeśli istnieje
        size_t len = strlen(wsk);
        if (len > 0 && wsk[len - 1] == '\n') {
            wsk[len - 1] = '\0';
        }

        sem_post(id_sem0);         // daj znać readerowi, że dane gotowe
    } while (wsk[0] != '!');

    // Sprzątanie
    munmap(wsk, ROZM_BLOKU);
    close(shm_fd);
    // Nie unlinkujemy semaforów ani pamięci — zrobi to reader

    sem_close(id_sem0);
    sem_close(id_sem1);

    return 0;
}
