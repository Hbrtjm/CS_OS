#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

void cleanup()
{
	unlink("interval");
	exit(0);
}

double function(double x) {
	return 4.0 / (x * x + 1);
}

void integrate_dx(double a, double dx, double *result) {
	*result = (function(a) + function(a + dx)) / 2 * dx;
}

double integrate(int k, double a, double b) {
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		perror("Cannot open the unnamed pipe");
		exit(1);
	}

	double dx = (b - a) / k;

	for (int i = 0; i < k; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			close(pipefd[0]);
			double local_result;
			integrate_dx(a + i * dx, dx, &local_result);
			write(pipefd[1], &local_result, sizeof(double));
			close(pipefd[1]);
			exit(0);
		} else if (pid < 0) {
			perror("Forking error");
			exit(1);
		}
	}

	close(pipefd[1]);

	double total = 0;
	for (int i = 0; i < k; i++) {
		double partial;
		if (read(pipefd[0], &partial, sizeof(double)) == sizeof(double)) {
			total += partial;
		} else {
			perror("Error while reading from the pipe");
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
		fprintf(stderr, "Usage: %s <number_of_subintervals>\n", argv[0]);
		return 1;
	}
	
	int k = atoi(argv[1]);
	
	signal(SIGINT,cleanup);
	
	if (mkfifo("interval", 0777) == -1) {
		perror("Error with mkfifo creation (might already exist)");
	}

	while (1) {
		int fd = open("interval", O_RDONLY);
		if (fd == -1) {
			perror("Failed to open for reading");
			continue;
		}

		double a, b;
		if (read(fd, &a, sizeof(double)) != sizeof(double) ||
			read(fd, &b, sizeof(double)) != sizeof(double)) {
			printf("Failed to read interval.\n");
			close(fd);
			continue;
		}
		close(fd);

		double integralResult = integrate(k, a, b);
		printf("Value of the integral on interval [%lf;%lf]: %lf\nSending it now\n", a, b, integralResult);

		fd = open("interval", O_WRONLY);
		if (fd == -1) {
			perror("Failed to open for writing");
			continue;
		}
		write(fd, &integralResult, sizeof(double));
		close(fd);
	}
	unlink("interval");
	return 0;
}
