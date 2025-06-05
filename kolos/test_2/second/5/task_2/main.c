#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>

#define BUF_SIZE 3

int buffer[BUF_SIZE];							/* bufor wspoldzielony */
int add=0;										/* indeks na dodanie kolejnego elementu */ 
int rem=0;										/* indeks do usunięcia kolejnego elementu */
int num=0;										/* liczba elementów w buforze */

pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;	
pthread_cond_t c_cons=PTHREAD_COND_INITIALIZER; 
pthread_cond_t c_prod=PTHREAD_COND_INITIALIZER; 

void *producer(void *param);
void *consumer(void *param);

int main (int argc, char *argv[])
{
	pthread_t tid1, tid2;		

	/* utworz watek producenta */
	if ( pthread_create(&tid1, NULL, producer, NULL) != 0) {
		fprintf (stderr, "Unable to create producer thread\n");
		exit (1);
	}
	/* utworz watek konsumenta */	
	if ( pthread_create(&tid2, NULL, consumer, NULL) != 0) {
		fprintf (stderr, "Unable to create consumer thread\n");
		exit (1);
	}
	/* poczekaj, aż utworzone wątki zakończa działanie */
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf ("Parent quiting\n");
	
	return 0;
}
 


void *producer(void *param)
{
	int i;
	for (i=1; i<=20; i++) 
	{
		/* Wstaw do bufora */
        	pthread_mutex_lock(&m);							
		if (num > BUF_SIZE) exit(1);	/* przepelnienie */
		while (num == BUF_SIZE)			/* blokuj, jeśli bufor jest pełny */
               		pthread_cond_wait(&c_prod, &m);
		/* jeśli wykonujesz kod tutaj, bufor nie jest pełny, więc dodaj element */
		buffer[add] = i;
		add = (add+1) % BUF_SIZE;
		num++;
        	pthread_cond_broadcast(&c_cons);
		pthread_mutex_unlock(&m);
		printf ("producer: inserted %d\n", i);  fflush (stdout);
	}
	printf ("producer quiting\n");  
	fflush (stdout);
}

/* Konsumuj wartosc(i);  */
void *consumer(void *param)
{
	int i;
    int kk = 0;
	while (1) {
        kk=kk+1	;	
        	pthread_mutex_lock(&m);							
		if (num < 0) exit(1);   /* niedomiar - underflow */
		while (num == 0)		/* blokuj, jeśli bufor jest pusty */
               		pthread_cond_wait(&c_cons, &m);
		/* jeśli kod wykonuje się tutaj, bufor nie jest pusty, więc usuń element */
		i = buffer[rem];
		rem = (rem+1) % BUF_SIZE;
		num--;
        	pthread_cond_broadcast(&c_prod);
		pthread_mutex_unlock(&m);
		printf ("Consume value %d\n", i);  fflush(stdout);
	
        if (kk >= 20) break;
	}
	printf ("consumer quiting\n");
	fflush (stdout);  	
}
