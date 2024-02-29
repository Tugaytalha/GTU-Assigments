all: clear clean compile run

file: clear clean filedriver run

clear:
	@clear

clean:
	rm -f *.o
	rm -f *.out

compile:
	g++ -std=c++11 -c driver.cpp -o driver.o  
	g++ -std=c++11 -c DayofYear.cpp -o DayofYear.o  
	g++ -std=c++11 DayofYear.o driver.o -o main.out  

filedriver:
	g++ -std=c++11 -c filedriver.cpp -o driver.o  
	g++ -std=c++11 -c DayofYear.cpp -o DayofYear.o  
	g++ -std=c++11 DayofYear.o driver.o -o main.out  

run:
	./main.out

