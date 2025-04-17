all: clean  build   run

args = "10"

clean:
	rm -f *.o *.out
	rm -f fifo1 fifo2

build:
	gcc -c -o main.o main.c -std=gnu99
	gcc -o main.out main.o

run:
	./main.out $(args)

clear:
	clear

delete:
	rm -f .txt





