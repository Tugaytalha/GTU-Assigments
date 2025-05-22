#!/usr/bin/env python3
# Chat and File Server implementation
# Multi-threaded, TCP-based chat and file-sharing system

import socket
import threading
import queue
import logging
import os
import sys
import time
import signal
import re
import json
from datetime import datetime

# Configuration constants
MAX_CLIENTS = 15
MAX_USERNAME_LEN = 16
MAX_ROOM_NAME_LEN = 32
MAX_UPLOAD_QUEUE = 5
MAX_FILE_SIZE = 3 * 1024 * 1024  # 3MB
ALLOWED_FILE_TYPES = ['.txt', '.pdf', '.jpg', '.png']
MAX_ROOM_CAPACITY = 15

# Global variables for server state
clients = {}
rooms = {}
client_rooms = {}  # Maps client_socket to current room
username_socket_map = {}
file_transfer_queue = queue.Queue(MAX_UPLOAD_QUEUE)
active_transfers = 0
active_transfers_lock = threading.Lock()

# Locks for synchronization
clients_lock = threading.Lock()
rooms_lock = threading.Lock()
client_rooms_lock = threading.Lock()
username_lock = threading.Lock()
file_queue_semaphore = threading.Semaphore(MAX_UPLOAD_QUEUE)
log_lock = threading.Lock()

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    handlers=[
        logging.FileHandler("server_log.txt"),
        logging.StreamHandler()
    ],
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger()

# Signal handler for graceful shutdown
def signal_handler(sig, frame):
    with log_lock:
        logger.info("[SHUTDOWN] SIGINT received. Disconnecting clients, saving logs.")
    
    # Notify all clients about server shutdown
    with clients_lock:
        for client_socket, username in clients.items():
            try:
                message = {"type": "server_shutdown", "message": "Server is shutting down. Goodbye!"}
                client_socket.send(json.dumps(message).encode('utf-8'))
            except:
                pass
            try:
                client_socket.close()
            except:
                pass
    
    sys.exit(0)

# Register signal handler
signal.signal(signal.SIGINT, signal_handler)

def validate_username(username):
    """Validate username (max 16 chars, alphanumeric only)"""
    if not username or len(username) > MAX_USERNAME_LEN:
        return False
    return bool(re.match(r'^[a-zA-Z0-9]+$', username))

def validate_room_name(room_name):
    """Validate room name (max 32 chars, alphanumeric only)"""
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

def broadcast_to_room(room_name, message, sender_socket=None):
    """Send message to all clients in a room except the sender"""
    with rooms_lock:
        if room_name not in rooms:
            return
        
        room_clients = rooms[room_name]
        
        with clients_lock:
            for client_socket in room_clients:
                # Don't send message back to sender
                if client_socket != sender_socket:
                    try:
                        client_socket.send(json.dumps(message).encode('utf-8'))
                    except:
                        # If sending fails, client will be removed on next heartbeat or command
                        pass

def handle_client_connection(client_socket, client_address):
    """Handle a client connection"""
    username = None
    
    try:
        # Wait for user to send a username
        data = client_socket.recv(1024).decode('utf-8')
        username_msg = json.loads(data)
        
        if username_msg["type"] != "login":
            raise Exception("First message must be a login")
        
        username = username_msg["username"]
        
        # Validate username
        if not validate_username(username):
            error_msg = {"type": "error", "message": "Invalid username. Must be alphanumeric and max 16 characters."}
            client_socket.send(json.dumps(error_msg).encode('utf-8'))
            return
        
        # Check for duplicate username
        with username_lock:
            if username in [name for name in username_socket_map.values()]:
                error_msg = {"type": "error", "message": "Username already taken. Choose another."}
                client_socket.send(json.dumps(error_msg).encode('utf-8'))
                with log_lock:
                    logger.info(f"[REJECTED] Duplicate username attempted: {username}")
                return
            
            # Add to username mapping
            username_socket_map[client_socket] = username
        
        # Add client to the clients dictionary
        with clients_lock:
            clients[client_socket] = username
        
        # Log successful connection
        with log_lock:
            logger.info(f"[CONNECT] New client connected: {username} from {client_address[0]}")
        
        # Send welcome message
        welcome_msg = {"type": "server", "message": f"Welcome to the chat server, {username}!"}
        client_socket.send(json.dumps(welcome_msg).encode('utf-8'))
        
        # Main client communication loop
        while True:
            try:
                data = client_socket.recv(1024).decode('utf-8')
                if not data:
                    break  # Client disconnected
                
                command = json.loads(data)
                handle_command(command, client_socket, username)
            except json.JSONDecodeError:
                # Invalid JSON sent by client
                error_msg = {"type": "error", "message": "Invalid message format"}
                client_socket.send(json.dumps(error_msg).encode('utf-8'))
            except Exception as e:
                # General error handling
                error_msg = {"type": "error", "message": f"Error processing command: {str(e)}"}
                client_socket.send(json.dumps(error_msg).encode('utf-8'))
    
    except Exception as e:
        # Handle connection errors
        if username:
            with log_lock:
                logger.error(f"[ERROR] Error handling client {username}: {str(e)}")
    
    finally:
        # Clean up when client disconnects
        cleanup_client(client_socket, username)

def handle_command(command, client_socket, username):
    """Process a client command"""
    cmd_type = command.get("type")
    
    if cmd_type == "join":
        room_name = command.get("room")
        join_room(client_socket, username, room_name)
    
    elif cmd_type == "leave":
        leave_room(client_socket, username)
    
    elif cmd_type == "broadcast":
        message = command.get("message")
        send_broadcast(client_socket, username, message)
    
    elif cmd_type == "whisper":
        target_user = command.get("target")
        message = command.get("message")
        send_whisper(client_socket, username, target_user, message)
    
    elif cmd_type == "sendfile":
        filename = command.get("filename")
        target_user = command.get("target")
        file_data = command.get("file_data")
        file_size = command.get("file_size")
        handle_file_transfer(client_socket, username, filename, target_user, file_data, file_size)
    
    elif cmd_type == "exit":
        # Client is exiting - cleanup will happen in the finally block
        success_msg = {"type": "server", "message": "Disconnected. Goodbye!"}
        client_socket.send(json.dumps(success_msg).encode('utf-8'))
        with log_lock:
            logger.info(f"[DISCONNECT] Client {username} disconnected.")
        raise Exception("Client exited")
    
    else:
        # Unknown command
        error_msg = {"type": "error", "message": "Unknown command"}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))

def join_room(client_socket, username, room_name):
    """Join a room or create it if it doesn't exist"""
    # Validate room name
    if not validate_room_name(room_name):
        error_msg = {"type": "error", "message": "Invalid room name. Must be alphanumeric and max 32 characters."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Check if client is already in a room
    current_room = None
    with client_rooms_lock:
        current_room = client_rooms.get(client_socket)
    
    if current_room:
        # Leave current room first
        leave_room(client_socket, username)
    
    # Join the new room
    with rooms_lock:
        if room_name not in rooms:
            # Create new room
            rooms[room_name] = set()
        
        # Check room capacity
        if len(rooms[room_name]) >= MAX_ROOM_CAPACITY:
            error_msg = {"type": "error", "message": f"Room '{room_name}' is full (max {MAX_ROOM_CAPACITY} users)."}
            client_socket.send(json.dumps(error_msg).encode('utf-8'))
            return
        
        # Add client to room
        rooms[room_name].add(client_socket)
    
    # Update client's current room
    with client_rooms_lock:
        client_rooms[client_socket] = room_name
    
    # Send confirmation to client
    success_msg = {"type": "server", "message": f"You joined the room '{room_name}'"}
    client_socket.send(json.dumps(success_msg).encode('utf-8'))
    
    # Notify others in the room
    room_msg = {"type": "room_notification", "message": f"{username} has joined the room."}
    broadcast_to_room(room_name, room_msg, client_socket)
    
    # Log the action
    with log_lock:
        logger.info(f"[JOIN] user '{username}' joined room '{room_name}'")

def leave_room(client_socket, username):
    """Leave the current room"""
    # Check if client is in a room
    current_room = None
    with client_rooms_lock:
        current_room = client_rooms.get(client_socket)
    
    if not current_room:
        error_msg = {"type": "error", "message": "You are not in any room."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Remove client from room
    with rooms_lock:
        if current_room in rooms:
            rooms[current_room].discard(client_socket)
            
            # If room is now empty, remove it
            if not rooms[current_room]:
                del rooms[current_room]
    
    # Update client's current room
    with client_rooms_lock:
        del client_rooms[client_socket]
    
    # Send confirmation to client
    success_msg = {"type": "server", "message": f"You left the room '{current_room}'"}
    client_socket.send(json.dumps(success_msg).encode('utf-8'))
    
    # Notify others in the room
    room_msg = {"type": "room_notification", "message": f"{username} has left the room."}
    broadcast_to_room(current_room, room_msg)
    
    # Log the action
    with log_lock:
        logger.info(f"[LEAVE] user '{username}' left room '{current_room}'")

def send_broadcast(client_socket, username, message):
    """Send a broadcast message to the current room"""
    # Check if client is in a room
    current_room = None
    with client_rooms_lock:
        current_room = client_rooms.get(client_socket)
    
    if not current_room:
        error_msg = {"type": "error", "message": "You are not in any room."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Create message to broadcast
    broadcast_msg = {
        "type": "broadcast", 
        "username": username, 
        "message": message, 
        "room": current_room
    }
    
    # Send message to all clients in the room
    broadcast_to_room(current_room, broadcast_msg, client_socket)
    
    # Send confirmation to sender
    success_msg = {"type": "server", "message": f"Message sent to room '{current_room}'"}
    client_socket.send(json.dumps(success_msg).encode('utf-8'))
    
    # Log the action
    with log_lock:
        logger.info(f"[BROADCAST] user '{username}': {message}")

def send_whisper(client_socket, username, target_user, message):
    """Send a private message to a specific user"""
    # Find the target user's socket
    target_socket = None
    with clients_lock:
        for sock, name in clients.items():
            if name == target_user:
                target_socket = sock
                break
    
    if not target_socket:
        error_msg = {"type": "error", "message": f"User '{target_user}' not found."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Create whisper message
    whisper_msg = {
        "type": "whisper",
        "from": username,
        "message": message
    }
    
    # Send message to target user
    try:
        target_socket.send(json.dumps(whisper_msg).encode('utf-8'))
    except:
        error_msg = {"type": "error", "message": f"Failed to send message to {target_user}."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Send confirmation to sender
    success_msg = {"type": "server", "message": f"Whisper sent to {target_user}"}
    client_socket.send(json.dumps(success_msg).encode('utf-8'))
    
    # Log the action
    with log_lock:
        logger.info(f"[WHISPER] user '{username}' sent whisper to '{target_user}'")

def process_file_transfer():
    """Process file transfers from the queue"""
    global active_transfers
    
    while True:
        # Get file transfer request from queue
        transfer_request = file_transfer_queue.get()
        
        with active_transfers_lock:
            active_transfers += 1
        
        try:
            # Extract transfer data
            sender_socket = transfer_request["sender_socket"]
            sender_username = transfer_request["sender_username"]
            target_username = transfer_request["target_username"]
            filename = transfer_request["filename"]
            file_data = transfer_request["file_data"]
            queue_time = transfer_request["queue_time"]
            
            # Calculate wait time
            wait_time = time.time() - queue_time
            
            # Find target socket
            target_socket = None
            with clients_lock:
                for sock, name in clients.items():
                    if name == target_username:
                        target_socket = sock
                        break
            
            if not target_socket:
                error_msg = {"type": "error", "message": f"User '{target_username}' not found or disconnected."}
                sender_socket.send(json.dumps(error_msg).encode('utf-8'))
                with log_lock:
                    logger.info(f"[FILE] Failed to send '{filename}' from '{sender_username}' to '{target_username}': User not found")
                continue
            
            # Check for filename collision and rename if needed
            base_name, ext = os.path.splitext(filename)
            new_filename = filename
            counter = 1
            
            # Create file transfer message
            file_msg = {
                "type": "file",
                "from": sender_username,
                "filename": new_filename,
                "file_data": file_data
            }
            
            # Send file to target
            try:
                target_socket.send(json.dumps(file_msg).encode('utf-8'))
            except:
                error_msg = {"type": "error", "message": f"Failed to send file to {target_username}."}
                sender_socket.send(json.dumps(error_msg).encode('utf-8'))
                with log_lock:
                    logger.info(f"[FILE] Failed to send '{filename}' from '{sender_username}' to '{target_username}': Connection error")
                continue
            
            # Send confirmation to sender
            success_msg = {"type": "server", "message": f"File sent successfully to {target_username}"}
            sender_socket.send(json.dumps(success_msg).encode('utf-8'))
            
            # Log the action with wait time
            with log_lock:
                logger.info(f"[FILE] '{filename}' from user '{sender_username}' started upload after {wait_time:.1f} seconds in queue.")
                logger.info(f"[SEND FILE] '{filename}' sent from {sender_username} to {target_username} (success)")
        
        except Exception as e:
            with log_lock:
                logger.error(f"[ERROR] Error processing file transfer: {str(e)}")
        
        finally:
            with active_transfers_lock:
                active_transfers -= 1
            
            # Mark task as done
            file_transfer_queue.task_done()

def handle_file_transfer(client_socket, username, filename, target_user, file_data, file_size):
    """Handle a file transfer request"""
    # Validate file
    valid, error_msg_text = is_valid_file(filename, file_size)
    if not valid:
        error_msg = {"type": "error", "message": error_msg_text}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        with log_lock:
            logger.info(f"[ERROR] File '{filename}' from user '{username}' exceeds size limit.")
        return
    
    # Check if target user exists
    target_exists = False
    with clients_lock:
        for name in clients.values():
            if name == target_user:
                target_exists = True
                break
    
    if not target_exists:
        error_msg = {"type": "error", "message": f"User '{target_user}' not found."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        return
    
    # Create transfer request
    transfer_request = {
        "sender_socket": client_socket,
        "sender_username": username,
        "target_username": target_user,
        "filename": filename,
        "file_data": file_data,
        "queue_time": time.time()
    }
    
    try:
        # Add file transfer to queue
        file_transfer_queue.put(transfer_request, block=False)
        
        # Calculate queue size
        queue_size = file_transfer_queue.qsize() + active_transfers
        
        # Send confirmation to client
        success_msg = {"type": "server", "message": f"File added to the upload queue. Position: {queue_size}"}
        client_socket.send(json.dumps(success_msg).encode('utf-8'))
        
        # Log the action
        with log_lock:
            logger.info(f"[FILE-QUEUE] Upload '{filename}' from {username} added to queue. Queue size: {queue_size}")
    
    except queue.Full:
        # Queue is full
        error_msg = {"type": "error", "message": "Upload queue is full. Try again later."}
        client_socket.send(json.dumps(error_msg).encode('utf-8'))
        with log_lock:
            logger.info(f"[FILE-QUEUE] Upload queue full. Rejected file '{filename}' from {username}")

def cleanup_client(client_socket, username):
    """Clean up resources when a client disconnects"""
    if not username:
        return
    
    # Remove from current room if any
    current_room = None
    with client_rooms_lock:
        current_room = client_rooms.get(client_socket)
        if current_room:
            del client_rooms[client_socket]
    
    if current_room:
        with rooms_lock:
            if current_room in rooms:
                rooms[current_room].discard(client_socket)
                
                # If room is now empty, remove it
                if not rooms[current_room]:
                    del rooms[current_room]
                else:
                    # Notify others in the room
                    room_msg = {"type": "room_notification", "message": f"{username} has left the room (disconnected)."}
                    broadcast_to_room(current_room, room_msg)
    
    # Remove from clients dictionary
    with clients_lock:
        if client_socket in clients:
            del clients[client_socket]
    
    # Remove from username mapping
    with username_lock:
        if client_socket in username_socket_map:
            del username_socket_map[client_socket]
    
    # Close socket
    try:
        client_socket.close()
    except:
        pass
    
    # Log the disconnection
    with log_lock:
        logger.info(f"[DISCONNECT] user '{username}' lost connection. Cleaned up resources.")

def start_server(port):
    """Start the chat server"""
    try:
        # Create server socket
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('0.0.0.0', port))
        server_socket.listen(5)
        
        with log_lock:
            logger.info(f"[INFO] Server listening on port {port}...")
        
        # Start file transfer processing thread
        file_thread = threading.Thread(target=process_file_transfer)
        file_thread.daemon = True
        file_thread.start()
        
        # Main server loop
        while True:
            # Accept new client connection
            client_socket, client_address = server_socket.accept()
            
            # Create a new thread to handle the client
            client_thread = threading.Thread(target=handle_client_connection, args=(client_socket, client_address))
            client_thread.daemon = True
            client_thread.start()
    
    except Exception as e:
        with log_lock:
            logger.error(f"[ERROR] Server error: {str(e)}")
    
    finally:
        # Cleanup
        try:
            server_socket.close()
        except:
            pass

if __name__ == "__main__":
    # Check command line arguments
    if len(sys.argv) != 2:
        print("Usage: ./chatserver <port>")
        sys.exit(1)
    
    try:
        port = int(sys.argv[1])
        if port < 1024 or port > 65535:
            print("Port must be between 1024 and 65535")
            sys.exit(1)
        
        start_server(port)
    except ValueError:
        print("Port must be a number")
        sys.exit(1)
