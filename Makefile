# Makefile for glusterfs-lock-demo

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -g -pthread

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET := $(BIN_DIR)/glusterfs-lock-demo

# Default target
all: $(TARGET)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean