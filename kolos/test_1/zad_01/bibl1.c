#include <stdlib.h>
#include <stdio.h>
#include "bibl1.h"

/*napisz biblioteke ladowana dynamicznie przez program zawierajaca funkcje:

1) zliczajaca sume n elementow tablicy tab:
int sumuj(int *tab, int n);

2) wyznaczajaca mediane n elementow tablicy tab
double mediana(int *tab, int n);

*/

int sumuj(int *tab, int n)
{
	int result = 0;
	for(int i = 0;i < n;i++)
	{
		result += tab[i];
	}
	return result;
}

double mediana(int *tab, int n)
{
	if(n % 2 == 0)
	{
		// return 1;
		return tab[(int)(n/2)];
	}
	else
	{
		// return 1;
		return ((double)tab[(int)(n/2)] + (double)tab[(int)(n/2+1)]) / 2;	
	}
}
