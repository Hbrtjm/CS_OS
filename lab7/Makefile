CC=gcc
CFLAGS=-Wall -std=c17 -D_XOPEN_SOURCE=500
# CFLAGS=-std=c17

BLDDIR=./build
SRCDIR=./src

.PHONY: all clean prepare

all: printers_posix clients_posix clients_systemv printers_systemv

prepare:
	mkdir -p build

posix: 
	$(CC) $(CFLAGS) $(SRCDIR)/printers_posix.c -lrt -lpthread -o $(BLDDIR)/printers_posix
	$(CC) $(CFLAGS) $(SRCDIR)/clients_posix.c -lrt -lpthread -o $(BLDDIR)/clients_posix

systemv: 
	$(CC) $(CFLAGS) $(SRCDIR)/printers_systemv.c -o $(BLDDIR)/printers_systemv
	$(CC) $(CFLAGS) $(SRCDIR)/clients_systemv.c -o $(BLDDIR)/clients_systemv

printers_posix: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/printers_posix.c -lrt -lpthread -o $(BLDDIR)/printers_posix
       	
clients_posix: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/clients_posix.c -lrt -lpthread -o $(BLDDIR)/clients_posix

printers_systemv: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/printers_systemv.c -o $(BLDDIR)/printers_systemv
       	
clients_systemv: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/clients_systemv.c -o $(BLDDIR)/clients_systemv

clean:
	rm -rf build
