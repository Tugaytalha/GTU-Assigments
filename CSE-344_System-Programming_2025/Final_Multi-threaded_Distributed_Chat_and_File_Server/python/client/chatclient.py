#!/usr/bin/env python3
# Chat Client implementation
# Multi-threaded, TCP-based chat and file-sharing system

import socket
import threading
import json
import sys
import os
import base64
import time
import colorama
from colorama import Fore, Style

# Initialize colorama for cross-platform colored terminal output
colorama.init()

# Configuration constants
MAX_USERNAME_LEN = 16
MAX_ROOM_NAME_LEN = 32
MAX_FILE_SIZE = 3 * 1024 * 1024  # 3MB
ALLOWED_FILE_TYPES = ['.txt', '.pdf', '.jpg', '.png']

# Global variables
client_socket = None
username = None
current_room = None
exit_flag = threading.Event()

def validate_username(username):
    """Validate username (max 16 chars, alphanumeric only)"""
    import re
    if not username or len(username) > MAX_USERNAME_LEN:
        return False
    return bool(re.match(r'^[a-zA-Z0-9]+$', username))

def validate_room_name(room_name):
    """Validate room name (max 32 chars, alphanumeric only)"""
    import re
    if not room_name or len(room_name) > MAX_ROOM_NAME_LEN:
        return False
    return bool(re.match(r'^[a-zA-Z0-9]+$', room_name))

def is_valid_file(filename, file_size):
    """Check if file is valid (type and size)"""
    _, ext = os.path.splitext(filename)
    if ext.lower() not in ALLOWED_FILE_TYPES:
        return False, "Invalid file type. Allowed types: " + ", ".join(ALLOWED_FILE_TYPES)
    if file_size > MAX_FILE_SIZE:
        return False, f"File too large. Maximum size: {MAX_FILE_SIZE/1024/1024}MB"
    return True, ""

def encode_file(file_path):
    """Read and encode a file in base64"""
    try:
        with open(file_path, 'rb') as file:
            file_data = file.read()
            file_size = len(file_data)
            
            # Check file validity
            valid, error_msg = is_valid_file(file_path, file_size)
            if not valid:
                return None, None, error_msg
            
            # Encode file data
            encoded_data = base64.b64encode(file_data).decode('utf-8')
            return encoded_data, file_size, None
    except FileNotFoundError:
        return None, None, "File not found."
    except Exception as e:
        return None, None, f"Error reading file: {str(e)}"

def send_message(message_dict):
    """Send a message to the server"""
    try:
        client_socket.send(json.dumps(message_dict).encode('utf-8'))
        return True
    except Exception as e:
        print(f"{Fore.RED}Error sending message: {str(e)}{Style.RESET_ALL}")
        return False

def receive_messages():
    """Receive and process messages from the server"""
    global current_room, username
    
    while not exit_flag.is_set():
        try:
            data = client_socket.recv(1024*1024*4).decode('utf-8')  # 4MB buffer for file transfers
            if not data:
                print(f"{Fore.RED}Connection to server lost{Style.RESET_ALL}")
                exit_flag.set()
                break
            
            message = json.loads(data)
            message_type = message.get("type")
            
            if message_type == "server":
                print(f"{Fore.GREEN}[Server]: {message['message']}{Style.RESET_ALL}")
                
                # Update room status if message indicates joining a room
                if "joined the room" in message['message']:
                    room_name = message['message'].split("'")[1]
                    current_room = room_name
            
            elif message_type == "error":
                print(f"{Fore.RED}[Server]: {message['message']}{Style.RESET_ALL}")
            
            elif message_type == "broadcast":
                sender = message["username"]
                msg = message["message"]
                room = message["room"]
                print(f"{Fore.CYAN}[{room}] {sender}: {msg}{Style.RESET_ALL}")
            
            elif message_type == "whisper":
                sender = message["from"]
                msg = message["message"]
                print(f"{Fore.MAGENTA}[Whisper from {sender}]: {msg}{Style.RESET_ALL}")
            
            elif message_type == "room_notification":
                print(f"{Fore.YELLOW}[Room]: {message['message']}{Style.RESET_ALL}")
            
            elif message_type == "file":
                sender = message["from"]
                filename = message["filename"]
                file_data = message["file_data"]
                
                # Decode file data
                try:
                    decoded_data = base64.b64decode(file_data)
                    
                    # Save file
                    if not os.path.exists("downloads"):
                        os.makedirs("downloads")
                    
                    # Handle filename collision
                    base_name, ext = os.path.splitext(filename)
                    save_path = os.path.join("downloads", filename)
                    counter = 1
                    
                    while os.path.exists(save_path):
                        new_filename = f"{base_name}_{counter}{ext}"
                        save_path = os.path.join("downloads", new_filename)
                        counter += 1
                    
                    with open(save_path, 'wb') as file:
                        file.write(decoded_data)
                    
                    print(f"{Fore.GREEN}[File received] {sender} sent you '{os.path.basename(save_path)}'. Saved to {save_path}{Style.RESET_ALL}")
                
                except Exception as e:
                    print(f"{Fore.RED}Error saving received file: {str(e)}{Style.RESET_ALL}")
            
            elif message_type == "server_shutdown":
                print(f"{Fore.RED}[Server]: {message['message']}{Style.RESET_ALL}")
                exit_flag.set()
                break
            
        except json.JSONDecodeError:
            print(f"{Fore.RED}Error: Received invalid data from server{Style.RESET_ALL}")
        except Exception as e:
            if not exit_flag.is_set():
                print(f"{Fore.RED}Error receiving messages: {str(e)}{Style.RESET_ALL}")
                exit_flag.set()
            break

def handle_command(command):
    """Handle user commands"""
    global current_room, username
    
    # Split the command into parts
    parts = command.split()
    
    if not parts:
        return
    
    cmd = parts[0].lower()
    
    # /join <room_name>
    if cmd == "/join":
        if len(parts) < 2:
            print(f"{Fore.RED}Usage: /join <room_name>{Style.RESET_ALL}")
            return
        
        room_name = parts[1]
        
        # Validate room name
        if not validate_room_name(room_name):
            print(f"{Fore.RED}Invalid room name. Must be alphanumeric and max 32 characters.{Style.RESET_ALL}")
            return
        
        # Send join command to server
        join_msg = {"type": "join", "room": room_name}
        send_message(join_msg)
    
    # /leave
    elif cmd == "/leave":
        if not current_room:
            print(f"{Fore.RED}You are not in any room.{Style.RESET_ALL}")
            return
        
        # Send leave command to server
        leave_msg = {"type": "leave"}
        send_message(leave_msg)
    
    # /broadcast <message>
    elif cmd == "/broadcast":
        if len(parts) < 2:
            print(f"{Fore.RED}Usage: /broadcast <message>{Style.RESET_ALL}")
            return
        
        if not current_room:
            print(f"{Fore.RED}You are not in any room.{Style.RESET_ALL}")
            return
        
        # Join all parts after command as message
        message = " ".join(parts[1:])
        
        # Send broadcast command to server
        broadcast_msg = {"type": "broadcast", "message": message}
        send_message(broadcast_msg)
    
    # /whisper <username> <message>
    elif cmd == "/whisper":
        if len(parts) < 3:
            print(f"{Fore.RED}Usage: /whisper <username> <message>{Style.RESET_ALL}")
            return
        
        target_user = parts[1]
        message = " ".join(parts[2:])
        
        # Send whisper command to server
        whisper_msg = {"type": "whisper", "target": target_user, "message": message}
        send_message(whisper_msg)
    
    # /sendfile <filename> <username>
    elif cmd == "/sendfile":
        if len(parts) != 3:
            print(f"{Fore.RED}Usage: /sendfile <filename> <username>{Style.RESET_ALL}")
            return
        
        filename = parts[1]
        target_user = parts[2]
        
        # Encode file
        file_data, file_size, error = encode_file(filename)
        if error:
            print(f"{Fore.RED}Error: {error}{Style.RESET_ALL}")
            return
        
        print(f"{Fore.YELLOW}Preparing to send file...{Style.RESET_ALL}")
        
        # Send file command to server
        file_msg = {
            "type": "sendfile",
            "filename": os.path.basename(filename),
            "target": target_user,
            "file_data": file_data,
            "file_size": file_size
        }
        send_message(file_msg)
    
    # /exit
    elif cmd == "/exit":
        print(f"{Fore.YELLOW}Disconnecting from server...{Style.RESET_ALL}")
        
        # Send exit command to server
        exit_msg = {"type": "exit"}
        send_message(exit_msg)
        
        # Set exit flag to stop receiving thread
        exit_flag.set()
    
    # Unknown command
    else:
        print(f"{Fore.RED}Unknown command. Type /help for available commands.{Style.RESET_ALL}")

def print_help():
    """Print help information"""
    print(f"{Fore.CYAN}===== Chat Client Help ====={Style.RESET_ALL}")
    print(f"{Fore.CYAN}/join <room_name>{Style.RESET_ALL} - Join or create a room")
    print(f"{Fore.CYAN}/leave{Style.RESET_ALL} - Leave the current room")
    print(f"{Fore.CYAN}/broadcast <message>{Style.RESET_ALL} - Send message to everyone in the room")
    print(f"{Fore.CYAN}/whisper <username> <message>{Style.RESET_ALL} - Send private message")
    print(f"{Fore.CYAN}/sendfile <filename> <username>{Style.RESET_ALL} - Send file to user")
    print(f"{Fore.CYAN}/exit{Style.RESET_ALL} - Disconnect from the server")
    print(f"{Fore.CYAN}/help{Style.RESET_ALL} - Show this help message")
    print(f"{Fore.CYAN}========================={Style.RESET_ALL}")

def main():
    """Main client function"""
    global client_socket, username, exit_flag
    
    # Check command line arguments
    if len(sys.argv) != 3:
        print("Usage: ./chatclient <server_ip> <port>")
        return
    
    server_ip = sys.argv[1]
    
    try:
        server_port = int(sys.argv[2])
    except ValueError:
        print("Port must be a number")
        return
    
    # Create client socket
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((server_ip, server_port))
    except Exception as e:
        print(f"Error connecting to server: {str(e)}")
        return
    
    print(f"{Fore.GREEN}Connected to server at {server_ip}:{server_port}{Style.RESET_ALL}")
    
    # Get username
    while True:
        username = input("Enter your username: ")
        if validate_username(username):
            break
        print(f"{Fore.RED}Invalid username. Must be alphanumeric and max 16 characters.{Style.RESET_ALL}")
    
    # Send login message
    login_msg = {"type": "login", "username": username}
    if not send_message(login_msg):
        return
    
    # Start thread to receive messages
    receive_thread = threading.Thread(target=receive_messages)
    receive_thread.daemon = True
    receive_thread.start()
    
    # Print welcome message and help
    print(f"{Fore.GREEN}Welcome to the chat client, {username}!{Style.RESET_ALL}")
    print_help()
    
    # Main input loop
    try:
        while not exit_flag.is_set():
            user_input = input("> ")
            
            if not user_input:
                continue
            
            if user_input.lower() == "/help":
                print_help()
                continue
            
            # Handle user command
            handle_command(user_input)
    
    except KeyboardInterrupt:
        print(f"{Fore.YELLOW}\nDisconnecting from server...{Style.RESET_ALL}")
        
        # Send exit command to server
        exit_msg = {"type": "exit"}
        try:
            send_message(exit_msg)
        except:
            pass
        
        exit_flag.set()
    
    except Exception as e:
        print(f"{Fore.RED}Error: {str(e)}{Style.RESET_ALL}")
    
    finally:
        # Clean up
        if client_socket:
            try:
                client_socket.close()
            except:
                pass
        
        print(f"{Fore.GREEN}Goodbye!{Style.RESET_ALL}")

if __name__ == "__main__":
    main()
