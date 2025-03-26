#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>   // So that we don't get 'implicit declaration' for exit()
#include <unistd.h>   // So that we don't get 'implicit declaration' for fork(), getpid()
#include <sys/wait.h> // So that we don't get 'implicit declaration' for wait()

int main(int argc, char *argv[]) {
    if (argc < 2) { // If it's bigger than 2 we ignore the other arguments
        printf("Usage: %s <number_of_processes>\n", argv[0]);
        return 1;
    }

    int num_processes = atoi(argv[1]);
    if (num_processes <= 0) {
        printf("Error: number_of_processes must be greater than 0.\n");
        return 2;
    }

    printf("Primary");
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            return 3;
        } else if (pid == 0) { 
            printf("Parent PID: %d, Child PID: %d\n", getppid(), getpid());
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    printf("%d\n", num_processes);
    return 0;
}

