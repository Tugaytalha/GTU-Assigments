CC = gcc
CFLAGS = -pthread -Wall -std=gnu99

all: clear clean main.o main 

main: main.o
	$(CC) $(CFLAGS) -o main main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o main

clear:
	clear

run:
	./main