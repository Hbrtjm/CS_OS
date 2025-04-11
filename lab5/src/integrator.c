#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

double function(double x) {
	return 4.0 / (x * x + 1);
}

double integrate_range(double start, double end, double dx) {
	double sum = 0.0;
	for (double x = start; x < end; x += dx) {
		sum += function(x) * dx;
	}
	return sum;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <dx> <n>\n", argv[0]);
		return 1;
	}

	double dx = atof(argv[1]);
	int n = atoi(argv[2]);

	if (dx <= 0 || n <= 0) {
		fprintf(stderr, "dx and n must be positive numbers.\n");
		return 1;
	}
	for (int k = 1; k <= n; k++) {
		int pipes[k][2];
		pid_t pids[k];

		struct timeval start_time, end_time;
		gettimeofday(&start_time, NULL);

		double interval = 1.0 / k;

		for (int i = 0; i < k; i++) {
			if (pipe(pipes[i]) == -1) {
				perror("pipe");
				exit(1);
			}

			pids[i] = fork();
			if (pids[i] == -1) {
				perror("fork");
				exit(1);
			}

			if (pids[i] == 0) {
				close(pipes[i][0]);

				double local_result = integrate_range(i * interval, (i + 1) * interval, dx);

				if (write(pipes[i][1], &local_result, sizeof(double)) != sizeof(double)) {
					perror("write");
				}

				close(pipes[i][1]);
				exit(0);
			}

			close(pipes[i][1]); 
		}

		double total_result = 0.0;

		for (int i = 0; i < k; i++) {
			double partial_result;
			if (read(pipes[i][0], &partial_result, sizeof(double)) != sizeof(double)) {
				perror("read");
			} else {
				total_result += partial_result;
			}
			close(pipes[i][0]);
		}

		for (int i = 0; i < k; i++) {
			waitpid(pids[i], NULL, 0);
		}

		gettimeofday(&end_time, NULL);
		double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
							  (end_time.tv_usec - start_time.tv_usec) / 1e6;

		printf("k = %d\t|\tResult = %.10f\t|\tTime = %.6f seconds\n", k, total_result, elapsed_time);
	}
	return 0;
}

