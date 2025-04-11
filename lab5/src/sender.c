#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	double a, b;

	while (1) {
		printf("Enter integration interval [a b] (or type 'q' to quit): ");
		char input[100];
		fgets(input, sizeof(input), stdin);
		if (input[0] == 'q' || input[0] == 'Q') break;

		if (sscanf(input, "%lf %lf", &a, &b) != 2) { // I found this here: https://stackoverflow.com/questions/10075294/converting-string-to-a-double-variable-in-c 
			printf("Invalid input. Please enter two numbers.\n");
			continue;
		}

		if (a > b) {
			printf("Value a (%lf) has to be smaller than b (%lf)\n", a, b);
			continue;
		}

		int fd = open("interval", O_WRONLY);
		if (fd == -1) {
			perror("Failed while opening for writing");
			continue;
		}

		write(fd, &a, sizeof(double));
		write(fd, &b, sizeof(double));
		close(fd);

		fd = open("interval", O_RDONLY);
		if (fd == -1) {
			perror("Failed while opening for reading");
			continue;
		}

		double result;
		if (read(fd, &result, sizeof(double)) == sizeof(double)) {
			printf("Result of the integral: %lf\n", result);
		} else {
			printf("Failed to read result from calculator\n");
		}
		close(fd);
	}

	return 0;
}

