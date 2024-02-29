all: clear clean driver1 driver2

clear:
	@clear

clean:
	rm -f *.o
	rm -f *.out

driver1:
	g++ -std=c++11 -c Tetromino.cpp -o Tetromino.o  
	g++ -std=c++11 -c AbstractTetris.cpp -o AbstractTetris.o  
	g++ -std=c++11 -c TetrisVector.cpp -o TetrisVector.o  
	g++ -std=c++11 -c TetrisArray1D.cpp -o TetrisArray1D.o  
	g++ -std=c++11 -c TetrisAdapter.tpp -o TetrisAdapter.o  
	g++ -std=c++11 -c Location.cpp -o Location.o  
	g++ -std=c++11 -c Tetris.cpp -o Tetris.o  
	g++ -std=c++11 -c driver1.cpp -o driver1.o  
	g++ -std=c++11 AbstractTetris.o Tetris.o TetrisVector.o Location.o TetrisArray1D.o Tetromino.o TetrisAdapter.o driver1.o -o main.out  
	./main.out

driver2:
	g++ -std=c++11 -c Tetromino.cpp -o Tetromino.o  
	g++ -std=c++11 -c AbstractTetris.cpp -o AbstractTetris.o  
	g++ -std=c++11 -c TetrisVector.cpp -o TetrisVector.o  
	g++ -std=c++11 -c TetrisArray1D.cpp -o TetrisArray1D.o  
	g++ -std=c++11 -c TetrisAdapter.tpp -o TetrisAdapter.o  
	g++ -std=c++11 -c Location.cpp -o Location.o  
	g++ -std=c++11 -c Tetris.cpp -o Tetris.o  
	g++ -std=c++11 -c driver2.cpp -o driver2.o  
	g++ -std=c++11 AbstractTetris.o Tetris.o TetrisVector.o Location.o TetrisArray1D.o Tetromino.o TetrisAdapter.o driver2.o -o main.out  
	./main.out

