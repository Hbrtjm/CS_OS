#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include "posix.h"
#include "grid.h"

typedef struct {
    int start_row;
    int end_row;
    char *foreground;
    char *background;
    int width;
    int height;
    bool active;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ThreadData;


void* worker_function(void *arg) {
    ThreadData *data = (ThreadData*) arg;

    while (1) {
        pthread_mutex_lock(&data->mutex);
        while (!data->active)
            pthread_cond_wait(&data->cond, &data->mutex);
        data->active = false;
        pthread_mutex_unlock(&data->mutex);

        for (int y = data->start_row; y < data->end_row; y++) {
            for (int x = 0; x < data->width; x++) {
                int idx = y * data->width + x;
                data->background[idx] = is_alive(x,y, data->foreground);	
	    }
        }
    }
    return NULL;
}


pthread_t threads[MAX_THREADS];
ThreadData thread_data[MAX_THREADS];

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n > MAX_THREADS) n = MAX_THREADS;

    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr();

    int width = GRID_WIDTH;
    int height = GRID_HEIGHT;
    char *foreground = create_grid();
    char *background = create_grid();
    init_grid(foreground);

    // Podział wierszy między wątki
    int rows_per_thread = height / n;

    for (int i = 0; i < n; i++) {
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == n - 1) ? height : (i + 1) * rows_per_thread;
        thread_data[i].width = width;
        thread_data[i].height = height;
        thread_data[i].active = false;
        pthread_mutex_init(&thread_data[i].mutex, NULL);
        pthread_cond_init(&thread_data[i].cond, NULL);

        pthread_create(&threads[i], NULL, worker_function, &thread_data[i]);
    }

    while (true) {
        draw_grid(foreground);

        for (int i = 0; i < n; i++) {
            thread_data[i].foreground = foreground;
            thread_data[i].background = background;
            pthread_mutex_lock(&thread_data[i].mutex);
            thread_data[i].active = true;
            pthread_cond_signal(&thread_data[i].cond);
            pthread_mutex_unlock(&thread_data[i].mutex);
        }

        usleep(200 * 1000);

        // Zamień plansze
        char *tmp = foreground;
        foreground = background;
        background = tmp;
    }

    endwin();
    destroy_grid(foreground);
    destroy_grid(background);
    return 0;
}

