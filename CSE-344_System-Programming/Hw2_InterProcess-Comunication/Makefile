CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
TARGET = ipc_daemon

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET) fifo1 fifo2

.PHONY: all clean 