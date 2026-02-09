CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = parking_system
BUILD_DIR = build
SRC_DIR = src

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/utils/utils.c \
       $(SRC_DIR)/utils/queue.c \
       $(SRC_DIR)/core/graph.c \
       $(SRC_DIR)/core/floor.c \
       $(SRC_DIR)/core/parking_system.c

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Create build directory structure
$(shell mkdir -p $(BUILD_DIR)/utils $(BUILD_DIR)/core)

# Main target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with: ./$(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Rebuild
rebuild: clean $(TARGET)

.PHONY: clean run rebuild
