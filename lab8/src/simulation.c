#define _GNU_SOURCE
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "grid.h"
#include "posix.h"

pthread_t threads[MAX_THREADS];
ThreadDataTypeDef thread_data[MAX_THREADS];
int n_threads;
volatile sig_atomic_t ready = 0;

void sigusr1_handler(int sig) {
}

void* worker_function(void *arg) {
	ThreadDataTypeDef *data = (ThreadDataTypeDef*)arg;

	signal(SIGUSR1, sigusr1_handler); // Register the handler

	while (1) {
		pause(); // Wait for SIGUSR1

		for (int y = data->start_row; y < data->end_row; y++)  
		{		
			// Calculate for each row in this thread if the cells in it are dead or alive 
			for (int x = 0; x < data->width; x++)  
			{ 
					int idx = y * data->width + x; 
					data->background[idx] = is_alive(y,x, data->foreground); 
			} 
		} 
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s <threads>\n", argv[0]);
		return 1;
	}

	n_threads = atoi(argv[1]);
	if (n_threads > MAX_THREADS) n_threads = MAX_THREADS;

	// For randomness
	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr();

	// Grid dimensions and two states of the grid - one dispalyed and the to be displayed state
	int width = GRID_WIDTH;
	int height = GRID_HEIGHT;
	char *foreground = create_grid();
	char *background = create_grid();
	init_grid(foreground);

	int rows_per_thread = height / n_threads;

	for (int i = 0; i < n_threads; i++) {
		thread_data[i].id = i;
		thread_data[i].start_row = i * rows_per_thread;
		thread_data[i].end_row = (i == n_threads - 1) ? height : (i + 1) * rows_per_thread;
		thread_data[i].width = width;
		thread_data[i].height = height;

		pthread_create(&threads[i], NULL, worker_function, &thread_data[i]);
	}

	while (true) {
		draw_grid(foreground);

		// For each thread create a data container
		for (int i = 0; i < n_threads; i++) {
			thread_data[i].foreground = foreground;
			thread_data[i].background = background;

			// Wake the thread up
			pthread_kill(threads[i], SIGUSR1);
		}

		usleep(50 * 1000);

		// Display the calculated background
		char *tmp = foreground;
		foreground = background;
		background = tmp;
	}

	// Cleanup, I should again create a function cleanup() and trap the exit signals
	endwin();
	destroy_grid(foreground);
	destroy_grid(background);
	return 0;
}

