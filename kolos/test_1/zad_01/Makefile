CC=gcc
CFLAGS=-Wall -fPIC -I.

LIBDIR=lib
BINDIR=bin

.PHONY: all clean test

all: libbuild main test 

prepare:
	mkdir -p lib
	mkdir -p bin

test:
	LD_LIBRARY_PATH=./lib ./$(BINDIR)/main

libbuild: bibl1.c bibl1.h | prepare
	$(CC) $(CFLAGS) -shared -o ./$(LIBDIR)/libbibl1.so bibl1.c 

main: zad1.c
	$(CC) $(CFLAGS) -o ./$(BINDIR)/main zad1.c -ldl	

clean:
	rm -rf ./lib ./bin

