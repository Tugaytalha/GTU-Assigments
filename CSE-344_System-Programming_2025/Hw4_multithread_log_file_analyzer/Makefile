CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

# Define the target executable
TARGET = LogAnalyzer

# Define source files
SOURCES = main.c buffer.c utils.c

# Define object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJECTS)

# Phony targets
.PHONY: all clean 