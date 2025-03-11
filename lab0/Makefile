# Standard C compiler for Unix systems
CC=gcc
# Flags:
# -std=c17 	- Setting the standard to C17 
# -W(arning)all - Enables all compiler's warning messages
# -g		- Makes easy debugging possible with gdb
# -c		- Compile the source files without linking
#
FLAGS=-std=c17 -Wall -g -c 

# Just made it look nice and organized
SRC_DIR=src
BUILD_DIR=build

# PHONY, synonym to false, tells the make 
.PHONY: all clean

# We input the make targets from below
all: countdown 

countdown:
	# -p To prevent an error if the directory exists
	mkdir -p $(BUILD_DIR)
	# We create an object file
	$(CC) $(FLAGS) $(SRC_DIR)/countdown.c -o $(BUILD_DIR)/countdown.o
	# Compile object file to the executable 
	$(CC) $(BUILD_DIR)/countdown.o -o $(BUILD_DIR)/countdown

clean:
	# Remove the build directory with recursive (-r) and force (-f) flags
	rm -rf $(BUILD_DIR)
