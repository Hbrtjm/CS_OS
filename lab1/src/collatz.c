#include "collatz.h"
#include <stdio.h>

int collatz_conjecture(int input) {
    if (input % 2 == 0) {
        return input / 2;
    } else {
        return 3 * input + 1;
    }
}

int test_collatz_convergence(int input, int max_iter, int *steps) {
    int count = 0;
    int current = input;

    while (current != 1 && count < max_iter) {
        steps[count++] = current;
        current = collatz_conjecture(current);
    }
    if (current == 1) {
        steps[count++] = 1; 
    }
    if(steps[count-1] != 1)
    {
        return 0;
    }
    return count;
}

