CC = gcc
CFLAGS = -Wall -Wextra -g

# Source files
SRCS = main.c bitboard.c definitions.c movegen.c hashkeys.c
# Object files
OBJS = $(SRCS:.c=.o)
# Output executable
TARGET = chess_engine

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
