#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Napisać Makefile służący do kompilowania pliku main.c

/*
 * Funkcja 'spawn_fib' powinna utworzyć proces potomny i wywołać w nim
 * program 'main' podając jako argument liczbę 'n'.
 */
void spawn_fib(int n) {
	int pid = fork();
	switch (pid)
	{
		case 0:
			char c[3];
			if(n < 10)
			{
				c[0] = (char)(n + '0');	
			}
			else
			{
				c[0] = (char)((int)(n / 10) + '0');
				c[1] = (char)( ( n % 10 ) + '0');
			}
			execl("./main", "main", c, (char*)NULL);
			break;
		case -1:
			perror("Error with spawn");
		default:
			break;
	}
}

int get_child_code(void) {
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;
    }
}

int fib(int n) {
    if (n == 0 || n == 1) {
        return 1;
    } else {
        spawn_fib(n - 1);
        spawn_fib(n - 2);
        return get_child_code() + get_child_code();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fputs("Usage: ./main N\n", stderr);
        exit(-1);
    }
    int n = atoi(argv[1]);
    if (n > 11 || n < 0) {
        fprintf(stderr, "Argument out of range: %d\n", n);
        exit(-1);
    } else {
        return fib(n);
    }
}
