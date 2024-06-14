CC = gcc
CFLAGS = -Wall
TARGET = client

all: $(TARGET)

$(TARGET): client.o pide_shop.h
	$(CC) $(CFLAGS) -o $(TARGET) client.o

client.o: client.c pide_shop.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o $(TARGET)
