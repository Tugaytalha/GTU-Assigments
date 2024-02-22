all: clean clear compile run

clear: 
	@clear
	
compile: main.c
	@echo "-------------------------------------------"
	@echo "Compiling..."
	@gcc -o test main.c -lm

run: test
	@echo "-------------------------------------------"
	@echo "Running the tests...."
	@echo "================================================================================="
	@./test
	@echo "================================================================================="
	@echo "Completed tests...."

clean: 
	@echo "-------------------------------------------"
	@echo "Removing compiled files..."
	@rm -f *.o
	@rm -f test
	@rm -f Temp.txt

