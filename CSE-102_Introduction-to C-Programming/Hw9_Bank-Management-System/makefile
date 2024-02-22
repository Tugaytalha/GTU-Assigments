all: clean compile run

compile: main.c
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
	@echo "\nFOR CLEAN TXT FILES USE make delData COMMAND\n\n"

delData:
	@rm -f *.txt
