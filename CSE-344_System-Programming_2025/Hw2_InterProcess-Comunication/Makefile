CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu11 # -g for debugging, Wall/Wextra for warnings
TARGET = main
FIFO1 = /tmp/hw2_fifo1
FIFO2 = /tmp/hw2_fifo2
LOGFILE = /tmp/daemon.log

# Default target
all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)
	@echo "$(TARGET) compiled successfully."

# Rule to run the program (example usage)
run: all
	@echo "Running $(TARGET)... Example: ./$(TARGET) 15 8"
	./$(TARGET) 15 8
	@echo "Check output above and log file: $(LOGFILE)"
	@echo "Waiting for processes to finish (approx $(CHILD_SLEEP_DURATION) seconds)..."
	@sleep $(shell expr $(CHILD_SLEEP_DURATION) + 3) # Wait a bit longer than child sleep
	@echo "Run finished."

# Rule to clean generated files
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(FIFO1) $(FIFO2) $(LOGFILE) *.o core.*
	@echo "Cleanup complete."

.PHONY: all clean run