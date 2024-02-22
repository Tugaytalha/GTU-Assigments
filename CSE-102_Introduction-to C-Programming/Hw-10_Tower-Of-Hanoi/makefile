all: clean compile run

compile: main.c 
	@echo "FOR CLEAN TXT FILES USE make delData COMMAND\n\n"
	@echo "-------------------------------------------"
	@echo "Compiling..."
	@gcc -o test main.c 

run:
	@echo "-------------------------------------------"
	@echo "Running the tests...."
	@echo "================================================================================="
	./test
	@echo "================================================================================="
	@echo "Completed tests...."

clean:
	@echo "-------------------------------------------"
	@echo "Removing compiled files..."
	@rm -f *.o

delData:
	@rm -f *.txt