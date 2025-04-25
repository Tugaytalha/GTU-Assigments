CC      = cc
CFLAGS  = -std=c17 -Wall -Wextra -pedantic -O2 -fsanitize=address,undefined
LDFLAGS = -pthread -lrt

OBJ_SRV = server.o teller.o
OBJ_CL  = client.o

all: BankServer BankClient

BankServer: $(OBJ_SRV)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

BankClient: $(OBJ_CL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c bank_common.h teller.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o BankServer BankClient AdaBank.bankLog /tmp/bankcli_* /tmp/SRV_FIFO

.PHONY: all clean
