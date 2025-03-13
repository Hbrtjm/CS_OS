#include <dlfcn.h>
#include <stdio.h>

int main() {
    	void *handle = dlopen("libcollatz.so", RTLD_LAZY);
    	if(!handle)
    	{
	    	printf("The library could not be opened");
		return 1;
    	}

    	int (*test_collatz)(int input, int max_depth, int *steps);
    	test_collatz = (int (*)(int input, int max_depth, int *steps))dlsym(handle,"test_collatz_convergence");

    	printf("Testing Collatz conjecture using a static library\n");
	int available_steps, tested_number;
	printf("Input the tested numer: ");
	scanf("%d", &tested_number);
	printf("Input the amount of steps: ");
	scanf("%d", &available_steps);
    	int steps[available_steps];
    	int amount_of_steps =     (*test_collatz)(tested_number, available_steps, steps);

	if (amount_of_steps > 0) {
		printf("Managed after %d steps:\n", amount_of_steps);
	}
       	else 
	{
		printf("Sequence did not converge\n");
	    	amount_of_steps = available_steps;
	}
	for (int i = 0; i < amount_of_steps; i++) {
		printf("%d ", steps[i]);
	}
	printf("\n");
	
	dlclose(handle);
}
