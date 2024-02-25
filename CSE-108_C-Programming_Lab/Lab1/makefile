all: compile run clear clean

compile: main.c
	@echo "Compiling......."
	@gcc -o main.o main.c
	@gcc -o main.out main.c
	@echo "Compiled."
	
run:
	@echo "\nRunning Main.......\n"
	@./main.out
	
clear:
	@clear
	
clean:
	@rm main.o
	@rm main.out
	@echo "Cleaned.\n"
