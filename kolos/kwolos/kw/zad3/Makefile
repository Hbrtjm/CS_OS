CC=gcc
CFLAGS=-I. -Wall -pthread 
DEPS = buffer.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o  
	gcc -o main -Wall -pthread  main.c -I.

clean:
	rm main *.o