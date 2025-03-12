CC=gcc
CFLAGS=-std=c17 -Wall -Werror -Wextra -g -fPIC -Isrc

SRC_DIR=src
BUILD_DIR=build
LIBNAME=libcollatz
OBJECTS=$(BUILD_DIR)/collatz.o

.PHONY: all clean

all: libstatic libshared client_static client_shared client_dynamic

# Same as before, but here we will not have to copy-paste for every make
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create object files of the given library sources
$(OBJECTS): $(SRC_DIR)/collatz.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/collatz.c -o $(OBJECTS)

# Compile into a *.a file
libstatic: $(OBJECTS)
	ar rcs $(BUILD_DIR)/$(LIBNAME).a $(OBJECTS)

# Shared library
libshared: $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$(LIBNAME).so $(OBJECTS)

# Compile the program with a library by linking it using previously generted *.a files 
client_static: $(SRC_DIR)/client_static.c libstatic
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/client_static $(SRC_DIR)/client_static.c $(BUILD_DIR)/$(LIBNAME).a

# Here we basically have the same code for client, but just to see separate two programs  
client_shared: $(SRC_DIR)/client_shared.c libshared
	$(CC) -o $(BUILD_DIR)/client_shared $(SRC_DIR)/client_shared.c -L$(BUILD_DIR) -lcollatz -Wl,-rpath=./$(BUILD_DIR)

# Here we have to load the library by overriding the DYNAMIC environmental variable using -D DYNAMIC=1, we show where the library is located $(BUILD_DIR), and where we should r(e)path the linking 
client_dynamic: $(SRC_DIR)/client_dynamic.c libshared
	$(CC) -o $(BUILD_DIR)/client_dynamic $(SRC_DIR)/client_dynamic.c -D DYNAMIC=1 -L$(BUILD_DIR) -lcollatz -Wl,-rpath=./$(BUILD_DIR) -lcollatz

# Remove the entire build, just as before
clean:
	rm -rf $(BUILD_DIR)

