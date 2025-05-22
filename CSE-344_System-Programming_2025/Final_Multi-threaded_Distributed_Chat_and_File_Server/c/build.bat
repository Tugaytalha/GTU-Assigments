@echo off
echo Building server...
gcc -Wall -Wextra -pthread -o chatserver server/main.c server/chatserver.c server/chatserver_client.c server/chatserver_commands.c server/chatserver_rooms.c server/chatserver_files.c server/chatserver_logging.c

echo Building client...
gcc -Wall -Wextra -pthread -o chatclient client/chatclient.c

echo Done.
