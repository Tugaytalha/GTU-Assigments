CC = gcc
CFLAGS = -Wall
TARGET = client
TARGET2 = HungryVeryMuch

all: $(TARGET) $(TARGET2)

$(TARGET): client.o pide_shop.h
	$(CC) $(CFLAGS) -o $(TARGET) client.o

$(TARGET2): HungryVeryMuch.o pide_shop.h
	$(CC) $(CFLAGS) -o $(TARGET2) HungryVeryMuch.o

client.o: client.c pide_shop.h
	$(CC) $(CFLAGS) -c client.c

HungryVeryMuch.o: HungryVeryMuch.c pide_shop.h
	$(CC) $(CFLAGS) -c HungryVeryMuch.c

clean:
	rm -f *.o $(TARGET)
