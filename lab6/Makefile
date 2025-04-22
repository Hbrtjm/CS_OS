CC=gcc
CFLAGS=-Wall -lrt -pedantic

BUILDDIR=build
SRCDIR=src

.PHONY: all clean


all: server_posix client_posix server_systemv client_systemv

prepare:
	rm -f interval
	mkdir -p $(BUILDDIR)

server_posix: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/server_posix $(SRCDIR)/server_posix.c
	
client_posix: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/client_posix $(SRCDIR)/client_posix.c

server_systemv: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/server_systemv $(SRCDIR)/server_systemv.c

client_systemv: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/client_systemv $(SRCDIR)/client_systemv.c

clean:
	rm -rf build
