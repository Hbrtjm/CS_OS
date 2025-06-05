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
    // POSIX named semaphores
    sem_t *id_sem0 = sem_open("nazwa_sem0", O_CREAT, 0666, 0);
    sem_t *id_sem1 = sem_open("nazwa_sem1", O_CREAT, 0666, 0);
    if (id_sem0 == SEM_FAILED || id_sem1 == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Create shared memory object
    int shm_fd = shm_open("/pamiec_wspolna", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Set size of shared memory
    if (ftruncate(shm_fd, ROZM_BLOKU) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map shared memory into address space
    char *wsk = mmap(NULL, ROZM_BLOKU, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (wsk == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Communication loop
    wsk[0] = 0;
    while (wsk[0] != '!') {
        sem_post(id_sem1);        // signal writer
        sem_wait(id_sem0);        // wait for writer to finish
        if (wsk[0] != '!') printf("%s\n", wsk);
    }

    // Cleanup
    munmap(wsk, ROZM_BLOKU);
    close(shm_fd);
    shm_unlink("/pamiec_wspolna");

    sem_close(id_sem0);
    sem_close(id_sem1);
    sem_unlink("nazwa_sem0");
    sem_unlink("nazwa_sem1");

    return 0;
}
