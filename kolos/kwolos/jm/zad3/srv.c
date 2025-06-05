#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define L_SLOW 8
#define PORT 12345  // musi pasować do klienta

int main(void) {
    int status, gniazdo, dlugosc, gniazdo2, lbajtow, i;
    struct sockaddr_in ser, cli;
    char buf[200];
    char pytanie[L_SLOW] = "abcdefgh";
    char odpowiedz[L_SLOW][10] = {
        "alfa", "bravo", "charlie", "delta", 
        "echo", "foxtrot", "golf", "hotel"
    };

    // Stwórz gniazdo TCP
    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo < 0) {
        perror("socket");
        return 1;
    }

    // Przypisz numer portu i interfejs 127.0.0.1
    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(PORT);
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    status = bind(gniazdo, (struct sockaddr*)&ser, sizeof(ser));
    if (status < 0) {
        perror("bind");
        return 1;
    }

    // Utwórz kolejkę oczekujących (maksymalnie 10)
    status = listen(gniazdo, 10);
    if (status < 0) {
        perror("listen");
        return 1;
    }

    printf("Serwer nasłuchuje na porcie %d...\n", PORT);

    while (1) {
        dlugosc = sizeof(cli);
        gniazdo2 = accept(gniazdo, (struct sockaddr*)&cli, (socklen_t*)&dlugosc);
        if (gniazdo2 == -1) {
            perror("accept");
            continue;
        }

        printf("Połączenie od: %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        lbajtow = 1;
        while (lbajtow > 0) {
            lbajtow = read(gniazdo2, buf, sizeof(buf));
            if (lbajtow > 0) {
                // Przetwarzaj tylko pierwszy znak
                for (i = 0; i < L_SLOW && pytanie[i] != buf[0]; i++);
                if (i < L_SLOW) {
                    write(gniazdo2, odpowiedz[i], strlen(odpowiedz[i]));
                }
            }
        }
        close(gniazdo2);
        printf("Zamknięto połączenie\n");
    }

    close(gniazdo);
    printf("KONIEC DZIAŁANIA SERWERA\n");
    return 0;
}
