CC = gcc
CFLAGS = -Wall -Wextra -pthread
SERVER_SRC_DIR = server
CLIENT_SRC_DIR = client
SERVER_SOURCES = $(SERVER_SRC_DIR)/main.c $(SERVER_SRC_DIR)/chatserver.c $(SERVER_SRC_DIR)/chatserver_client.c \
                $(SERVER_SRC_DIR)/chatserver_commands.c $(SERVER_SRC_DIR)/chatserver_rooms.c \
                $(SERVER_SRC_DIR)/chatserver_files.c $(SERVER_SRC_DIR)/chatserver_logging.c
CLIENT_SOURCES = $(CLIENT_SRC_DIR)/chatclient.c

all: server client

server: $(SERVER_SOURCES)
	$(CC) $(CFLAGS) -o chatserver $(SERVER_SOURCES)

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) -o chatclient $(CLIENT_SOURCES)

clean:
	rm -f chatserver chatclient

.PHONY: all server client clean
