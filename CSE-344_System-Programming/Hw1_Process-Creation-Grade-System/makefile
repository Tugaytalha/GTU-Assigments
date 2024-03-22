all: clean  build   run

clean:
	rm -f *.o *.out

build:
	gcc -c -o main.o main.c
	gcc -o main.out main.o

run:
	./main.out

delete:
	rm -f .txt