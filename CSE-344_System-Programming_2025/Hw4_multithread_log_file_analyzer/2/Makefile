CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

# Target executable
TARGET = LogAnalyzer

# Source files
SRCS = main.c buffer.c utils.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the program
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
