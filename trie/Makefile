# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -w
TARGET = trie
SRC_FILES = trie.c
GDB = -g

# Build target
all: $(TARGET)

# Compile the source files (main.c, exercise.c, munit.c) into the TARGET executable
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) $(GDB)

# Clean target (removes the executable)
clean:
	rm -f $(TARGET)

# Run the tests
run-tests: $(TARGET)
	./$(TARGET)

# PHONY ensures that these targets will run even if a file with the same name exists
.PHONY: all clean run-tests
