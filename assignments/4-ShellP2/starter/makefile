# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable name
TARGET = dsh

# Find all source and header files
SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)

# Default target
all: $(TARGET) copy_to_bats

# Compile source to executable
$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

copy_to_bats: $(TARGET)
	cp $(TARGET) bats/

# Clean up build files
clean:
	rm -f $(TARGET)
	rm -f bats/$(TARGET)

test:
	bats $(wildcard ./bats/*.sh)

valgrind:
	echo "pwd\nexit" | valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TARGET)
	echo "pwd\nexit" | valgrind --tool=helgrind --error-exitcode=1 ./$(TARGET)

# Phony targets
.PHONY: all clean test copy_to_bats
