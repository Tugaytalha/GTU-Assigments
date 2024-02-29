all: clear clean driver1

clear:
	@clear

clean:
	rm -f *.o
	rm -f *.out

driver1:
	g++ -std=c++2a -c out_bound_except.cpp -o out_bound_except.o  
	g++ -std=c++2a -c driver1.cpp -o driver1.o  
	g++ -std=c++2a out_bound_except.o driver1.o -o main.out  
	./main.out
