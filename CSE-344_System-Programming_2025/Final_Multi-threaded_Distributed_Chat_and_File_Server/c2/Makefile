CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -pthread

# Server objects
SERVER_OBJS = chatserver.o chatserver_client.o chatserver_room.o chatserver_commands.o chatserver_files.o chatserver_log.o

# Client objects
CLIENT_OBJS = chatclient.o

# All targets
all: chatserver chatclient

# Server executable
chatserver: $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Client executable
chatclient: $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Pattern rule for object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f *.o chatserver chatclient

# Install (optional)
install: all
	mkdir -p bin
	cp chatserver chatclient bin/

# Phony targets
.PHONY: all clean install
