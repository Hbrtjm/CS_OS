#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Too few arguments. Usage: %s <k>\n", argv[0]);
		return 1;
	}
	
	double a,b; 
	sscanf(argv[1], "%lf", &a);
	sscanf(argv[2], "%lf", &b);
	
	if(a > b)
	{
		printf("Value a (%lf) has to be smaller than b (%lf)\n", a, b);
		return 2;
	}
	
	int fd = open("interval", O_WRONLY);
	write(fd, &a, sizeof(double));
	write(fd, &b, sizeof(double));
	close(fd);
	
	
	return 0;
}

