/*
Napisz program w języku C, który:
Wyświetla swój PID na początku.
Rejestruje niestandardowe procedury obsługi sygnałów dla następujących sygnałów:
SIGINT (Ctrl+C)
SIGTERM (sygnał zakończenia)
SIGUSR1 (sygnał zdefiniowany przez użytkownika 1)
SIGUSR2 (sygnał zdefiniowany przez użytkownika 2)
Przechowuje i drukuje licznik dla każdego odebranego sygnału.
Po otrzymaniu 5 sygnałów SIGUSR1 program powinien wydrukować specjalną wiadomość (np. „Otrzymano 5 sygnałów SIGUSR1!”), ale kontynuować działanie.
Po otrzymaniu 3 sygnałów SIGUSR2 program powinien zakończyć się czysto z komunikatem wyjścia.
Ignoruje SIGPIPE (nie powinno nastąpić żadne działanie).
Wyświetla komunikat (np. „Wciąż działa...”) co 3 sekundy w pętli.
*/

/*
make
./main
# W innym terminalu:
kill -SIGUSR1 <pid>
kill -SIGUSR2 <pid>
kill -SIGINT <pid>
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

volatile sig_atomic_t sigusr1_count = 0;
volatile sig_atomic_t sigusr2_count = 0;

void handle_sigint(int sig) {
    printf("Received SIGINT (Ctrl+C)\n");
}

void handle_sigterm(int sig) {
    printf("Received SIGTERM - process will not terminate\n");
}

void handle_sigusr1(int sig) {
    sigusr1_count += 1;
    printf("SIGUSR1 %d",sigusr1_count);
    if (sigusr1_count == 5) {
       printf("Otrzymano 5 sygnałów SIGUSR1!");
    }
}

void handle_sigusr2(int sig) {
    sigusr2_count += 1;
    printf("SIGUSR2 %d", sigusr2_count);
    if (sigusr2_count == 3) {
	printf("Wyjście po 3 sygnałach SIGUSR2. Żegnaj!");
    	exit(0);
    }
}

void install_signal_handlers() {
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE
}

int main() {
    printf("Process PID: %d\n", getpid());
    printf("Waiting for signals...\n");

    install_signal_handlers();

    while (1) {
        //...
        printf("Still running...\n");
        sleep(3);
    }

    return 0;
}


/*
Oczekiwany wynik/zachowanie:
Po uruchomieniu program pokazuje:

PID procesu: 12345
Oczekiwanie na sygnały...

Jeśli użytkownik wysyła sygnały za pomocą kill -SIGUSR1 12345 itd., program powinien:

Wyświetlać komunikaty specyficzne dla sygnału.
Policzyć, ile sygnałów każdego rodzaju zostało odebranych.
Reagować odpowiednio po przekroczeniu progów (5 SIGUSR1, 3 SIGUSR2).

Po wyjściu (po 3 SIGUSR2) powinien pokazać:
Wyjście po 3 sygnałach SIGUSR2. Żegnaj!
*/
