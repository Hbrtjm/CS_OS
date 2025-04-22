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
	int new_tab[n];
       	for(int i = 0; i < n;i++)
	{
		new_tab[i] = tab[i];
	}	
	int tmp;
	for(int i = 0;i < n; i++)
	{
		for(int j = i; j < n;j++)
		{
			if(new_tab[i] > new_tab[j])
			{
				tmp = new_tab[i];
				new_tab[i] = new_tab[j];
				new_tab[j] = tmp;
			}
		}
	}
	if(n % 2 != 0)
	{
		// return 1;
		return new_tab[(int)(n/2)];
	}
	else
	{
		// return 1;
		return ((double)new_tab[(int)(n/2)] + (double)new_tab[(int)(n/2+1)]) / 2;	
	}
}
