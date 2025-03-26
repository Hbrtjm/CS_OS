CC=gcc
CFLAGS=-std=c17 -Wall -g -fPIC 

SRC_DIR=src
BUILD_DIR=build
.PHONY: all clean

all: flipper counter

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

unpack:
	mkdir -p $(BUILD_DIR)
	# Extract the zip file
	tar -xf art.tgz -C $(BUILD_DIR)

counter: $(SRC_DIR)/counter.c unpack | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/counter.c -o $(BUILD_DIR)/counter

flipper: $(SRC_DIR)/flipper.c unpack | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/flipper.c -o $(BUILD_DIR)/flipper 

# Remove the entire build, just as before
clean:
	rm -rf $(BUILD_DIR)

