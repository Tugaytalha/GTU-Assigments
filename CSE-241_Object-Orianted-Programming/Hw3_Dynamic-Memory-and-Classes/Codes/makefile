all: clear clean driver1 driver2

clear:
	@clear

clean:
	rm -f *.o
	rm -f *.out

driver1:
	g++ -std=c++11 -c driver1.cpp -o driver1.o  
	g++ -std=c++11 -c Tetromino.cpp -o Tetromino.o  
	g++ -std=c++11 -c Board.cpp -o Board.o  
	g++ -std=c++11 Board.o Tetromino.o driver1.o -o driver1.out  
	./driver1.out

driver2:
	g++ -std=c++11 -c driver2.cpp -o driver2.o  
	g++ -std=c++11 -c Tetromino.cpp -o Tetromino.o  
	g++ -std=c++11 -c Board.cpp -o Board.o  
	g++ -std=c++11 Board.o Tetromino.o driver2.o -o main.out  
	./main.out

