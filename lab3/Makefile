CC=gcc
CFLAGS=-std=c17 -Wall -g -fPIC 

SRC_DIR=src
BUILD_DIR=build

.PHONY: all clean

all: creator executor 

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

creator: $(SRC_DIR)/creator.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/creator.c -o $(BUILD_DIR)/creator

executor: $(SRC_DIR)/executor.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/executor.c -o $(BUILD_DIR)/executor 

# Remove the entire build, just as before
clean:
	rm -rf $(BUILD_DIR)

