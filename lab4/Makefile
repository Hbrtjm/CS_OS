CC=gcc
CFLAGS=-Wall -g
SRC=src
BUILD=build

.PHONY: all clean

all: handler catcher sender

build:
	mkdir -p build

handler: | build
	$(CC) $(CFLAGS) -o $(BUILD)/handler  $(SRC)/handler.c 

sender: | build
	$(CC) $(CFLAGS) -o $(BUILD)/sender $(SRC)/sender.c 

catcher: | build
	$(CC) $(CFLAGS) -o $(BUILD)/catcher  $(SRC)/catcher.c 

clean:
	rm -rf build
