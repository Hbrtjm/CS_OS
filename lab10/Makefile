CC = gcc
CFLAGS = -Wall -g -std=c17

SRCDIR = src
BLDDIR = build

.PHONY: all clean

all: client server 
	
prepare:
	mkdir -p ./$(BLDDIR)

client: | prepare 
	$(CC) $(CFLAGS) $(SRCDIR)/common.c $(SRCDIR)/client.c -o $(BLDDIR)/client

server: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/common.c $(SRCDIR)/server.c -o $(BLDDIR)/server

clean:
	rm -rf $(BLDDIR)
