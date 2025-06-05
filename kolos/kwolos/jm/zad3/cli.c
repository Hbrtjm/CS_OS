#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define L_PYTAN 10
#define PORT 12345  // <- Make sure this matches the server's port

int main(void) {
    int status, gniazdo, i;
    struct sockaddr_in ser;
    char buf[200];
    char pytanie[] = "abccbahhhh";

    // Stwórz gniazdo TCP
    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo < 0) {
        perror("socket");
        return 1;
    }

    // Skonfiguruj adres serwera
    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(PORT);  // Port ustalony przez serwer
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Nawiąż połączenie
    status = connect(gniazdo, (struct sockaddr*)&ser, sizeof(ser));
    if (status < 0) {
        perror("connect");
        return 1;
    }

    // Wysyłanie po jednym znaku i odbieranie odpowiedzi
    for (i = 0; i < L_PYTAN; i++) {
        status = write(gniazdo, pytanie + i, 1);
        if (status < 0) {
            perror("write");
            break;
        }

        // Odbiór danych
        status = read(gniazdo, buf, sizeof(buf) - 1);
        if (status < 0) {
            perror("read");
            break;
        }

        buf[status] = '\0';  // null-terminate for safe printing
        printf("%s", buf);
    }

    printf("\n");

    close(gniazdo);
    printf("KONIEC DZIALANIA KLIENTA\n");
    return 0;
}
