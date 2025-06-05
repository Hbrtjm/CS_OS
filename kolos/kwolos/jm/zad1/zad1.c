#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define L_SLOW 7

pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;
char slownik[L_SLOW][10] = {
    "alfa", "bravo", "charlie", "delta", "echo", "foxtrot", "golf"
};
int NR = 0;

void* fun_watka(void* parametr) {
    // Lock, print the next word, advance & wrap NR
    pthread_mutex_lock(&mutex01);
      printf("%s ", slownik[NR]);
      fflush(stdout);
      NR++;
      if (NR >= L_SLOW) NR = 0;
    pthread_mutex_unlock(&mutex01);

    sleep(1);
    return NULL;
}

int main(void) {
    const int N = 20;
    pthread_t threads[N];

    // Create N threads
    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, fun_watka, NULL);
    }

    // Join all N threads
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n");
    return 0;
}
