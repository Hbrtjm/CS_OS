CC = gcc
CFLAGS = -Wall -g -std=c17

SRCDIR = src
BLDDIR = build

.PHONY: all clean

all: life simulation test
	
prepare:
	# rm ./$(SRCDIR)/grid.*
	# rm ./$(SRCDIR)/life.*
	mkdir -p ./$(BLDDIR)
	# tar -zxf life.tar.gz $(SRCDIR)
	# rm ./$(SRCDIR)/Makefile

life: | prepare
	$(CC) $(CFLAGS) $(SRCDIR)/grid.c $(SRCDIR)/life.c -o $(BLDDIR)/life -lncursesw
	
simulation: | prepare 
	$(CC) $(CFLAGS) $(SRCDIR)/grid.c $(SRCDIR)/simulation.c -o $(BLDDIR)/simulation -lpthread -lncursesw

test:
	./$(BLDDIR)/simulation 1

clean:
	rm -f life
	rm -rf $(BLDDIR)
