CC = gcc
CFLAGS = -Wall -pthread
TARGET = PideShop

all: $(TARGET)

$(TARGET): main.o server.o cook.o delivery.o utils.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o server.o cook.o delivery.o utils.o

main.o: main.c pide_shop.h
	$(CC) $(CFLAGS) -c main.c

server.o: server.c pide_shop.h
	$(CC) $(CFLAGS) -c server.c

cook.o: cook.c pide_shop.h
	$(CC) $(CFLAGS) -c cook.c

delivery.o: delivery.c pide_shop.h
	$(CC) $(CFLAGS) -c delivery.c

utils.o: utils.c pide_shop.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o $(TARGET)
