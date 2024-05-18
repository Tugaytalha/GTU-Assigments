CC = gcc
CFLAGS = -pthread -Wall

all: clear clean main.c main 

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