CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -fsanitize=address
LDFLAGS = -lm

INCLUDE_DIR = include
SRC_DIR = src
BIN_DIR = bin
TEST_DIR = test

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)
	rm -rf $(BIN_DIR)/*.o

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN_DIR)/*