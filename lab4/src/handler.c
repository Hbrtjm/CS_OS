#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void handler(int signum)
{
    printf("Handler\n");
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("No arguments supplied\n");
        return 1;
    }

    sigset_t new_mask, old_mask, pending;

    if (strcmp(argv[1], "none") == 0)
    {
        signal(SIGUSR1, SIG_DFL);
    }
    else if (strcmp(argv[1], "ignore") == 0)
    {
        signal(SIGUSR1, SIG_IGN);
    }
    else if (strcmp(argv[1], "handler") == 0)
    {
        signal(SIGUSR1, handler);
    }
    else if (strcmp(argv[1], "mask") == 0)
    {
        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    }
    else
    {
        printf("Provided argument is invalid\n");
        return 2;
    }

    printf("Process PID: %d\n", getpid());

    raise(SIGUSR1);

    if (strcmp(argv[1], "mask") == 0)
    {
        sigpending(&pending);
        if (sigismember(&pending, SIGUSR1))
        {
            printf("SIGUSR1 is pending\n");
        }

        sigprocmask(SIG_SETMASK, &old_mask, NULL);
    }

    while (1)
    {
        pause();
    }

    return 0;
}

