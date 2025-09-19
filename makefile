# ---------------- Configuration ----------------
CC       := gcc
CSTD     := -std=c17
WARN     := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
OPT      := -O3
DBG      := -g
INCLUDES := -Iinclude

SRC_DIR  := src
BLD_DIR  := build

# ---------------- Targets ----------------
ENGINE_TARGET := chess_engine
# ADAPTER_TARGET := perft_adapter # Target for the perftree script
# # --- Add all other executables you have created ---
# PERFT_TEST_TARGET := perft_test
# PERFT_DEBUGGER_TARGET := perft_debugger

# ---------------- Sources / Objects ----------------
# List all .c files that have their own main() function
MAIN_FILES := \
    $(SRC_DIR)/main.c
#     $(SRC_DIR)/perft_adapter.c \
#     $(SRC_DIR)/perft_test.c \
#     $(SRC_DIR)/perft_debugger.c \

# Common source files are ALL .c files EXCEPT the ones with a main() function
COMMON_SRCS := $(filter-out $(MAIN_FILES), $(wildcard $(SRC_DIR)/*.c))
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(COMMON_SRCS))

# Specific sources for each target
ENGINE_SRC := $(SRC_DIR)/main.c
ENGINE_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(ENGINE_SRC))

# ADAPTER_SRC := $(SRC_DIR)/perft_adapter.c
# ADAPTER_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(ADAPTER_SRC))

# PERFT_TEST_SRC := $(SRC_DIR)/perft_test.c
# PERFT_TEST_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(PERFT_TEST_SRC))

# PERFT_DEBUGGER_SRC := $(SRC_DIR)/perft_debugger.c
# PERFT_DEBUGGER_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(PERFT_DEBUGGER_SRC))


CFLAGS := $(CSTD) $(WARN) $(OPT) $(DBG) $(INCLUDES)

# ---------------- Build Rules ----------------
.PHONY: all clean run

# Build all targets
all: $(ENGINE_TARGET) $(ADAPTER_TARGET) $(PERFT_TEST_TARGET) $(PERFT_DEBUGGER_TARGET)

# Rule to build the main chess engine
$(ENGINE_TARGET): $(BLD_DIR) $(COMMON_OBJS) $(ENGINE_OBJ)
	$(CC) $(CFLAGS) $(COMMON_OBJS) $(ENGINE_OBJ) -o $@

# # Rule to build the perft adapter script
# $(ADAPTER_TARGET): $(BLD_DIR) $(COMMON_OBJS) $(ADAPTER_OBJ)
# 	$(CC) $(CFLAGS) $(COMMON_OBJS) $(ADAPTER_OBJ) -o $@

# # Rule to build the perft_test executable
# $(PERFT_TEST_TARGET): $(BLD_DIR) $(COMMON_OBJS) $(PERFT_TEST_OBJ)
# 	$(CC) $(CFLAGS) $(COMMON_OBJS) $(PERFT_TEST_OBJ) -o $@

# # Rule to build the perft_debugger executable
# $(PERFT_DEBUGGER_TARGET): $(BLD_DIR) $(COMMON_OBJS) $(PERFT_DEBUGGER_OBJ)
# 	$(CC) $(CFLAGS) $(COMMON_OBJS) $(PERFT_DEBUGGER_OBJ) -o $@


# Compile source files into object files
$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Utility Rules ---
run: $(ENGINE_TARGET)
	@./$(ENGINE_TARGET)

debug: CFLAGS := $(CSTD) $(WARN) -O0 -g3 $(INCLUDES)
debug: clean all

clean:
	@rm -rf $(BLD_DIR) $(ENGINE_TARGET) $(ADAPTER_TARGET) $(PERFT_TEST_TARGET) $(PERFT_DEBUGGER_TARGET)

$(BLD_DIR):
	@mkdir -p $@
