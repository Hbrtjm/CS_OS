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


void* worker_function(void *arg)
{
	// The worker function always takes argument as a void*
	ThreadDataTypeDef *data = (ThreadDataTypeDef*) arg;

	while (1)
	   	{
		// Lock the thread
		pthread_mutex_lock(&data->mutex);
		while (!data->active)
			pthread_cond_wait(&data->cond, &data->mutex);
		data->active = false;
		// Unlock the thread 
		pthread_mutex_unlock(&data->mutex);

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


pthread_t threads[MAX_THREADS];
ThreadDataTypeDef thread_data[MAX_THREADS]; // Information about each thread's values for each thread to know what they have to deal with

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

	int rows_per_thread = height / n;

	for (int i = 0; i < n; i++) {
		// Part the bord with respect to each row, every thread has constant width that it occupies on the board
		thread_data[i].start_row = i * rows_per_thread;
		thread_data[i].end_row = (i == n - 1) ? height : (i + 1) * rows_per_thread;
		thread_data[i].width = width;
		thread_data[i].height = height;
		thread_data[i].active = false;

		// Initialize the mutex for new thread
		pthread_mutex_init(&thread_data[i].mutex, NULL);
		// Initialize the start condition of the thread
		pthread_cond_init(&thread_data[i].cond, NULL);
		// Create a new thread
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

		usleep(1 * 1000);
	// Swap foreground the the background - the background is what will be displayed next after all of the threads calculated their respective pieces of the board 
		char *tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin();
	destroy_grid(foreground);
	destroy_grid(background);
	return 0;
}
