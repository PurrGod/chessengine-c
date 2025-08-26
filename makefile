# ---------------- Configuration ----------------
CC       := gcc
CSTD     := -std=c17
WARN     := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
OPT      := -O2
DBG      := -g
INCLUDES := -Iinclude

SRC_DIR  := src
INC_DIR  := include
TST_DIR  := tests
BLD_DIR  := build
BIN_DIR  := bin

TARGET   := $(BIN_DIR)/chess_engine
TESTBIN  := $(BIN_DIR)/tests

# ---------------- Sources / Objects ----------------
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(SRCS))

# Exclude main.o when linking tests
OBJS_NO_MAIN := $(filter-out $(BLD_DIR)/main.o,$(OBJS))

TEST_SRCS := $(wildcard $(TST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TST_DIR)/%.c,$(BLD_DIR)/%.test.o,$(TEST_SRCS))

CFLAGS := $(CSTD) $(WARN) $(OPT) $(DBG) $(INCLUDES)

# ---------------- Default targets ----------------
.PHONY: all clean run test debug

all: $(TARGET)

$(TARGET): $(BIN_DIR) $(BLD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile engine sources
$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------- Tests ----------------
$(TESTBIN): $(BIN_DIR) $(BLD_DIR) $(OBJS_NO_MAIN) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(OBJS_NO_MAIN) $(TEST_OBJS) -o $@

# Compile test sources
$(BLD_DIR)/%.test.o: $(TST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TESTBIN)
	@echo "Running tests..."
	@$(TESTBIN)

run: $(TARGET)
	@$(TARGET)

debug: CFLAGS := $(CSTD) $(WARN) -O0 -g3 $(INCLUDES)
debug: clean all

clean:
	@rm -rf $(BLD_DIR) $(BIN_DIR)

$(BIN_DIR) $(BLD_DIR):
	@mkdir -p $@
