CC=gcc
CFLAGS=-Wall

BUILDDIR=build
SRCDIR=src

.PHONY: all clean


all: integrator sender calculator

prepare:
	rm -f interval
	mkdir -p $(BUILDDIR)

integrator: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/integrator $(SRCDIR)/integrator.c
	
sender: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/sender $(SRCDIR)/sender.c

calculator: | prepare
	$(CC) $(CFLAGS) -o $(BUILDDIR)/calculator $(SRCDIR)/calculator.c

clean:
	rm -rf build
