# Makefile for compiling demonstration C files

# Compiler
CC = gcc

# Standard compiler flags
CFLAGS = -Wall -Wextra -g

# Source files
SOURCES = client1 server1 server2 server3 proto_test

# Executable names
EXECUTABLES = $(SOURCES:.c=)

# Default target: build all executables
all: $(EXECUTABLES)

# Rule for compilation with protocol.c
$(EXECUTABLES): %: %.c protocol.c
	$(CC) $(CFLAGS) -o $@ $^

# Clean up compiled files
clean:
	rm -f $(EXECUTABLES)

# Phony targets
.PHONY: all clean