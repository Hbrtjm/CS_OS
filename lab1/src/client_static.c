#include "collatz.h"
#include <stdio.h>
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

int main()
{
    	printf("Testing Collatz conjecture using a static library\n");
	int available_steps, tested_number;
	printf("Input the tested numer: ");
	scanf("%d", &tested_number);
	printf("Input the amount of steps: ");
	scanf("%d", &available_steps);
    	int steps[available_steps];
    	int amount_of_steps = test_collatz_convergence(tested_number, available_steps, steps);

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
	return 0;
}
