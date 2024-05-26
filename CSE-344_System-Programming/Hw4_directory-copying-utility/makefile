CC = gcc
CFLAGS = -pthread -Wall -Wextra
TARGET = MWCp

all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o: 210104004084_main.c
	$(CC) $(CFLAGS) -c 210104004084_main.c -o main.o

clean:
	rm -f *.o $(TARGET)
