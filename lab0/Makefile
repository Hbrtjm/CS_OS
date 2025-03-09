# Standard C compiler
CC=gcc
# Flags:
# -std=c17 	- Setting the standard to C17 
# -W(arning)all - Enables all compiler's warning messages
# -g		- Makes easy debugging possible with gdb
# -c		- To compiler the source files without linking
#
FLAGS=-std=c17 -Wall -g -c 

# Just made it look nice and organized
SRC_DIR=src
BUILD_DIR=build

.PHONY: all
all:
	# -p To prevent an error if the directory exists
	mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(SRC_DIR)/countdown.c -o $(BUILD_DIR)/countdown.o
	$(CC) $(BUILD_DIR)/countdown.o -o $(BUILD_DIR)/countdown
countdown:
	# -p To prevent an error if the directory exists
	mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(SRC_DIR)/countdown.c -o $(BUILD_DIR)/countdown.o
	$(CC) $(BUILD_DIR)/countdown.o -o $(BUILD_DIR)/countdown

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
