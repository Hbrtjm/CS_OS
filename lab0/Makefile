# Standard C compiler
CC=gcc
# Flags:
# -std=c17 	- Setting the standard to C17 
# -W(arning)all - Enables all compiler's warning messages
# -g		- Makes easy debugging possible with gdb
FLAGS=-std=c17 -Wall -g

# Just made it look nice and organized
SRC_DIR=src
BUILD_DIR=build


all:
	# -p To prevent an error if the directory exists
	mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(SRC_DIR)/countdown.c -o $(BUILD_DIR)/countdown

countdown:
	# -p To prevent an error if the directory exists
	mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(SRC_DIR)/countdown.c -o $(BUILD_DIR)/countdown

clear:
	rm -rf $(BUILD_DIR)
