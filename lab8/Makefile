CC = gcc
CFLAGS = -Wall -g -std=c17

SRCDIR = src
BLDDIR = build

.PHONY: all clean

all: life simulation
	
prepare:
	mkdir -p ./$(BLDDIR)

life: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/grid.c $(SRCDIR)/life.c -o $(BLDDIR)/life -lncursesw
	
simulation: | prepare 
	$(CC) $(CFLAGS) $(SRCDIR)/grid.c $(SRCDIR)/simulation.c -o $(BLDDIR)/simulation -lpthread -lncursesw

clean:
	rm -f life
