#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

double function(double x) {
	return 4 / (x * x + 1);
}

void integrate_dx(double a, double dx, double *result) {
	*result = (function(a) + function(a + dx)) / 2 * dx;
}

double integrate(int k, double a, double b) {
	int pipefd[2];
	int status = pipe(pipefd);
	if (status == -1) {
		perror("pipe");
		exit(1);
	}
	
	double dx = (b - a) / k;
	
	for (int i = 0; i < k; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			close(pipefd[0]);
			double local_result;
			integrate_dx(a + i * dx, dx, &local_result);
			
			if (write(pipefd[1], &local_result, sizeof(double)) != sizeof(double)) {
			    perror("write");
			}
			
			close(pipefd[1]);
			exit(0);
		} else if (pid < 0) {
			perror("fork");
			exit(1);
		}
	}

	close(pipefd[1]);
	
	double total = 0;
	for (int i = 0; i < k; i++) {
		double partial;
		int bytesRead = read(pipefd[0], &partial, sizeof(double));
		if (bytesRead == sizeof(double)) {
		    total += partial;
		} else {
			perror("read");
		}
	}

	close(pipefd[0]);

	for (int i = 0; i < k; i++) {
		wait(NULL);
	}
	
	return total;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Too few arguments. Usage: %s <k>\n", argv[0]);
		return 1;
	}
	
	int k = atoi(argv[1]);
	if (k <= 0) {
		fprintf(stderr, "k must be a positive\n");
		return 1;
	}
	
	double result = integrate(k, 0, 1);
	printf("Result of integration: %f\n", result);
	return 0;
}

