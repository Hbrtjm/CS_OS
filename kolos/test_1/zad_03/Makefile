CC=gcc
CFLAGS=-g

.PHONY: all clean

all: main

main:
	$(CC) $(CFLAGS) -o main main.c

test:
	chmod +x test.sh
	./test.sh

clean:
	rm -rf main
