CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic -std=c11 -pthread -O2
LDFLAGS := -pthread

all: hw3

hw3: main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f hw3
