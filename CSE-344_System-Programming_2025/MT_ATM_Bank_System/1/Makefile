CC = gcc
CFLAGS = -Wall -g -pthread
LDFLAGS = -lrt

# Primary targets
all: bank_server bank_client advanced_server

# Standard version
bank_server: bank_server.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

bank_client: bank_client.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Advanced version with shared memory and semaphores
advanced_server: advanced_server.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Object files
bank_server.o: bank_server.c common.h
	$(CC) $(CFLAGS) -c $<

bank_client.o: bank_client.c common.h
	$(CC) $(CFLAGS) -c $<

advanced_server.o: advanced_server.c common.h
	$(CC) $(CFLAGS) -c $<

# Clean
clean:
	rm -f *.o bank_server bank_client advanced_server

# Run targets
run_server:
	./bank_server AdaBank ServerFIFO_1

run_advanced:
	./advanced_server AdaBank ServerFIFO_1

run_client1:
	./bank_client Client01.file ServerFIFO_1

run_client2:
	./bank_client Client02.file ServerFIFO_1

run_client3:
	./bank_client Client03.file ServerFIFO_1

.PHONY: all clean run_server run_advanced run_client1 run_client2 run_client3 