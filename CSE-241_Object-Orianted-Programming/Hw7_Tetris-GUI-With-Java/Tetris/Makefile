all: clear clean compile run

run: run1 run2

compile: compile1 compile2

driver1: compile1 run1

driver2: compile2 run2

clear:
	@clear

clean:
	rm -f *.class

compile1: input.java Tetromino.java Tetris.java Location.java Driver1.java
	javac -d . Location.java input.java Tetromino.java Tetris.java Driver1.java 

compile2: input.java Tetromino.java Tetris.java Location.java Driver1.java Driver2.java
	javac -d . Location.java input.java Tetromino.java Tetris.java Driver2.java

run1: 
	java Tetris.Driver1

run2: 
	java Tetris.Driver2
	
