# ---------------- Configuration ----------------
CC       := gcc
CSTD     := -std=c17
WARN     := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
OPT      := -O2
DBG      := -g
INCLUDES := -Iinclude

SRC_DIR  := src
BLD_DIR  := build

# Change TARGET to the current directory
TARGET   := chess_engine

# ---------------- Sources / Objects ----------------
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(SRCS))

CFLAGS := $(CSTD) $(WARN) $(OPT) $(DBG) $(INCLUDES)

# ---------------- Default targets ----------------
.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(BLD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile sources
$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@./$(TARGET) # Run from the current directory

debug: CFLAGS := $(CSTD) $(WARN) -O0 -g3 $(INCLUDES)
debug: clean all

clean:
	@rm -rf $(BLD_DIR) $(TARGET)

$(BLD_DIR):
	@mkdir -p $@