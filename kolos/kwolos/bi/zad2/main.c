#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "/my_kol_sem"  // poprawka: nazwy semaforów POSIX zaczynają się od '/'

void print_sem_value(sem_t *sem){
    int sem_val;
    sem_getvalue(sem, &sem_val);
    printf("Current sem value: %d\n", sem_val);	
}

int main(int argc, char** args){

    if(argc != 4){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    /************************************
     Utwórz semafor POSIX-owy. Ustaw jego wartość na 1
    *************************************/
    sem_t *sem_id = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem_id == SEM_FAILED) {
        if (errno == EEXIST) {
            sem_unlink(SEM_NAME); // usuwamy stary semafor jeśli istnieje
            sem_id = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
        }
        if (sem_id == SEM_FAILED) {
            perror("sem_open failed");
            exit(1);
        }
    }

    print_sem_value(sem_id);

    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open file");
        return 1;
    }

    int parentLoopCounter = atoi(args[1]);
    int childLoopCounter = atoi(args[2]);
    int max_sleep_time = atoi(args[3]);

    char buf[128];
    pid_t childPid;

    if ((childPid = fork())) {
        // PARENT
        int status = 0;
        srand((unsigned)time(NULL));

        while (parentLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            print_sem_value(sem_id);

            /******************************************
             Sekcja krytyczna. Zabezpiecz dostęp semaforem
            ******************************************/
            sem_wait(sem_id);

            print_sem_value(sem_id);

            sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            print_sem_value(sem_id);

            /****************************************************
             Koniec sekcji krytycznej
            ****************************************************/
            sem_post(sem_id);

            print_sem_value(sem_id);
        }

        waitpid(childPid, &status, 0);
    } else {
        // CHILD
        srand((unsigned)(time(NULL) ^ getpid())); // inny seed

        while (childLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            print_sem_value(sem_id);

            /******************************************
             Sekcja krytyczna. Zabezpiecz dostęp semaforem
            ******************************************/
            sem_wait(sem_id);

            print_sem_value(sem_id);

            sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            print_sem_value(sem_id);

            /****************************************************
             Koniec sekcji krytycznej
            ****************************************************/
            sem_post(sem_id);

            print_sem_value(sem_id);
        }

        _exit(0);
    }

    /***************************************
     Posprzątaj semafor
    ***************************************/
    sem_close(sem_id);
    sem_unlink(SEM_NAME);
    close(fd);

    return 0;
}
