#–––– compiler & flags ––––
CC      ?= gcc                # override on the CLI if you prefer clang
CFLAGS  := -Wall -Wextra -pedantic -std=c11 -Iinclude
LDFLAGS := -pthread
LDLIBS_SERVER := -lrt         # POSIX shm / semaphores

#–––– source files ––––
SRC_SERVER := src/bank_server.c src/teller.c src/teller_lib.c src/utils.c
SRC_CLIENT := src/bank_client.c src/utils.c

OBJ_SERVER := $(SRC_SERVER:.c=.o)
OBJ_CLIENT := $(SRC_CLIENT:.c=.o)

BIN_SERVER := bank_server
BIN_CLIENT := bank_client

.PHONY: all server client clean run valgrind test

all: $(BIN_SERVER) $(BIN_CLIENT)

#–––– linking rules ––––
$(BIN_SERVER): $(OBJ_SERVER)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS_SERVER)

$(BIN_CLIENT): $(OBJ_CLIENT)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

#–––– pattern rule for objects ––––
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#–––– convenience targets ––––
run: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "Starting AdaBank server …"
	@./$(BIN_SERVER) &                       \
	SERVER_PID=$$!;                         \
	sleep 1;                                \
	echo "→ Running sample clients";        \
	./$(BIN_CLIENT) tests/client01.file &   \
	./$(BIN_CLIENT) tests/client02.file &   \
	wait;                                   \
	echo "→ Shutting server";               \
	kill -SIGTERM $$SERVER_PID;             \
	wait $$SERVER_PID

valgrind: $(BIN_SERVER)
	valgrind --leak-check=full --show-leak-kinds=all ./$(BIN_SERVER)

clean:
	$(RM) $(OBJ_SERVER) $(OBJ_CLIENT) $(BIN_SERVER) $(BIN_CLIENT) \
	       Adabank.fifo AdaBank.bankLog /tmp/adabank_*.fifo
