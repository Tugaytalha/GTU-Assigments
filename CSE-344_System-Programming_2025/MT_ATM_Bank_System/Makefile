CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic
LDFLAGS = -pthread                    # pthread needed by POSIX semaphores

all: BankServer BankClient

BankServer: bank_server.c teller.c teller_actions.c common.h teller.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

BankClient: bank_client.c common.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f BankServer BankClient AdaBank*.bankLog AdaBankFIFO *.o
.PHONY: all clean
