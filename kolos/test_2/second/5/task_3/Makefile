CFLAGS=-o
CC=gcc

.PHONY: all clean test

all: shm_write shm_read test

shm_write: 
	$(CC) $(CFLAGS) shm_write shm_write.c
shm_read: 
	$(CC) $(CFLAGS) shm_read shm_read.c
test:
	sudo ./shm_write
	./shm_read
clean:
	rm -rf shm_write shm_read
