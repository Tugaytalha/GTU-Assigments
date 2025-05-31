#!/usr/bin/env python3
"""
Test script for the multi-threaded chat and file server system.
This script tests various features and edge cases of the system.
"""

import subprocess
import threading
import socket
import time
import os
import sys
import random
import string
import signal
from pathlib import Path

# Configuration
SERVER_IP = '127.0.0.1'
SERVER_PORT = 8888
TEST_TIMEOUT = 60  # seconds
MAX_CLIENTS = 15
UPLOAD_DIR = 'uploads'
TEST_FILES_DIR = 'test_files'
TEST_USERS = [f'user{i}' for i in range(1, MAX_CLIENTS + 1)]
TEST_ROOMS = ['room1', 'room2', 'room3']

# Colors for output
GREEN = '\033[92m'
RED = '\033[91m'
YELLOW = '\033[93m'
BLUE = '\033[94m'
RESET = '\033[0m'

# Test state
server_process = None
client_processes = {}
test_results = []
test_lock = threading.Lock()


def setup_test_environment():
    """Prepare the test environment by creating necessary directories and files."""
    print(f"{BLUE}Setting up test environment...{RESET}")
    
    # Create test file directory
    os.makedirs(TEST_FILES_DIR, exist_ok=True)
    
    # Create upload directories for each test user
    for user in TEST_USERS:
        path = os.path.join(UPLOAD_DIR, user)
        os.makedirs(path, exist_ok=True)
    
    # Create test files of different types and sizes
    create_test_files()
    
    print(f"{GREEN}Test environment setup complete.{RESET}")


def create_test_files():
    """Create various test files for testing the file transfer capabilities."""
    files_to_create = [
        ('small.txt', 1024),          # 1 KB
        ('medium.txt', 1024 * 100),   # 100 KB
        ('large.txt', 1024 * 1024),   # 1 MB
        ('max.txt', 3 * 1024 * 1024 - 1000),  # Almost 3 MB
        ('oversized.txt', 3 * 1024 * 1024 + 1000),  # Over 3 MB
    ]
    
    for filename, size in files_to_create:
        filepath = os.path.join(TEST_FILES_DIR, filename)
        with open(filepath, 'wb') as f:
            f.write(os.urandom(size))
        print(f"Created test file: {filename} ({size} bytes)")
    
    # Copy test files to each user's upload directory
    for user in TEST_USERS:
        user_dir = os.path.join(UPLOAD_DIR, user)
        for filename, _ in files_to_create:
            src = os.path.join(TEST_FILES_DIR, filename)
            dst = os.path.join(user_dir, filename)
            with open(src, 'rb') as src_file, open(dst, 'wb') as dst_file:
                dst_file.write(src_file.read())


def start_server():
    """Start the chat server and return the process."""
    print(f"{BLUE}Starting chat server on port {SERVER_PORT}...{RESET}")
    try:
        process = subprocess.Popen(
            ['./chatserver', str(SERVER_PORT)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        # Wait a moment for the server to start
        time.sleep(2)
        
        # Check if the server started successfully
        if process.poll() is not None:
            print(f"{RED}Server failed to start!{RESET}")
            stdout, stderr = process.communicate()
            print(f"STDOUT: {stdout}")
            print(f"STDERR: {stderr}")
            return None
        
        print(f"{GREEN}Server started successfully.{RESET}")
        return process
    except Exception as e:
        print(f"{RED}Error starting server: {e}{RESET}")
        return None


def stop_server(process):
    """Stop the chat server gracefully."""
    if process and process.poll() is None:
        print(f"{BLUE}Stopping server...{RESET}")
        process.send_signal(signal.SIGINT)
        try:
            process.wait(timeout=5)
            print(f"{GREEN}Server stopped successfully.{RESET}")
        except subprocess.TimeoutExpired:
            print(f"{YELLOW}Server did not stop gracefully, terminating...{RESET}")
            process.terminate()
            process.wait()


def simulate_client(username, commands=None, delay=0.5):
    """Simulate a client with automated commands."""
    if commands is None:
        commands = []
    
    print(f"{BLUE}Starting client for user '{username}'...{RESET}")
    
    # Start client process
    process = subprocess.Popen(
        ['./chatclient', SERVER_IP, str(SERVER_PORT)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )
    
    # Store the process
    client_processes[username] = process
    
    # Write username
    time.sleep(1)
    process.stdin.write(f"{username}\n")
    process.stdin.flush()
    
    # Wait for login to complete
    time.sleep(2)
    
    # Execute each command with delay
    for cmd in commands:
        time.sleep(delay)
        process.stdin.write(f"{cmd}\n")
        process.stdin.flush()
        print(f"Client {username} executed: {cmd}")
    
    return process


def test_basic_functionality():
    """Test basic client connections, room joining, and messaging."""
    log_test_result("Basic Functionality Test", "Started")
    
    # Start 3 clients
    client1 = simulate_client("user1", [
        "/join room1",
        "/broadcast Hello from user1!",
        "This is a regular message in room1"
    ])
    
    time.sleep(1)
    
    client2 = simulate_client("user2", [
        "/join room1",
        "/broadcast I've joined room1!",
        "/whisper user1 This is a private message to user1"
    ])
    
    time.sleep(1)
    
    client3 = simulate_client("user3", [
        "/join room2",
        "/broadcast Hello from room2!",
        "/whisper user1 Can you see this from room2?"
    ])
    
    # Let the messages be processed
    time.sleep(5)
    
    # Check outputs (simplified - in a real test we'd check more thoroughly)
    stdout1, _ = read_process_output(client1)
    stdout2, _ = read_process_output(client2)
    stdout3, _ = read_process_output(client3)
    
    success = (
        "Hello from user1!" in stdout2 and
        "I've joined room1!" in stdout1 and
        "This is a private message to user1" in stdout1 and
        "Hello from room2!" not in stdout1 and  # Should not see messages from other rooms
        "Can you see this from room2?" in stdout1  # But should see whispers
    )
    
    log_test_result("Basic Functionality Test", "PASSED" if success else "FAILED")
    
    # Clean up clients
    for client in [client1, client2, client3]:
        client.stdin.write("/exit\n")
        client.stdin.flush()
        client.wait(timeout=2)


def test_duplicate_username():
    """Test rejection of duplicate usernames."""
    log_test_result("Duplicate Username Test", "Started")
    
    # Start first client
    client1 = simulate_client("duplicateuser", [])
    time.sleep(2)
    
    # Try to start a second client with the same username
    process = subprocess.Popen(
        ['./chatclient', SERVER_IP, str(SERVER_PORT)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )
    
    time.sleep(1)
    process.stdin.write("duplicateuser\n")
    process.stdin.flush()
    
    # Wait to see if it gets rejected
    time.sleep(3)
    stdout, _ = read_process_output(process)
    
    success = "already taken" in stdout.lower() or "in use" in stdout.lower()
    
    log_test_result("Duplicate Username Test", "PASSED" if success else "FAILED")
    
    # Clean up
    client1.stdin.write("/exit\n")
    client1.stdin.flush()
    process.stdin.write("/exit\n")
    process.stdin.flush()
    
    client1.wait(timeout=2)
    process.wait(timeout=2)


def test_file_transfer():
    """Test file transfer between clients."""
    log_test_result("File Transfer Test", "Started")
    
    # Ensure the download directory exists on the client side
    download_dir = Path('downloads')
    download_dir.mkdir(exist_ok=True)
    
    # Start sender and recipient
    sender = simulate_client("sender", [
        "/join transferroom",
        "Preparing to send a file"
    ])
    
    time.sleep(1)
    
    recipient = simulate_client("recipient", [
        "/join transferroom",
        "Ready to receive a file"
    ])
    
    time.sleep(2)
    
    # Send a file
    sender.stdin.write("/sendfile small.txt recipient\n")
    sender.stdin.flush()
    
    # Wait for transfer to complete
    time.sleep(10)
    
    # Check outputs
    stdout_sender, _ = read_process_output(sender)
    stdout_recipient, _ = read_process_output(recipient)
    
    # Check if file was mentioned in the outputs
    sender_success = "queued for transfer" in stdout_sender.lower() or "sending file" in stdout_sender.lower()
    recipient_success = "receiving file" in stdout_recipient.lower() or "file received" in stdout_recipient.lower()
    
    success = sender_success and recipient_success
    
    log_test_result("File Transfer Test", "PASSED" if success else "FAILED")
    
    # Clean up
    sender.stdin.write("/exit\n")
    sender.stdin.flush()
    recipient.stdin.write("/exit\n")
    recipient.stdin.flush()
    
    sender.wait(timeout=2)
    recipient.wait(timeout=2)


def test_room_capacity():
    """Test room capacity limits."""
    log_test_result("Room Capacity Test", "Started")
    
    room_name = "capacityroom"
    processes = []
    
    # Join 10 clients to the same room (assuming MAX_ROOM_CLIENTS is 10)
    for i in range(1, 11):
        username = f"capacity{i}"
        process = simulate_client(username, [f"/join {room_name}"])
        processes.append(process)
        time.sleep(0.5)
    
    # Try to join one more client
    overflow_client = simulate_client("capacityoverflow", [f"/join {room_name}"])
    processes.append(overflow_client)
    
    # Wait for responses
    time.sleep(5)
    
    # Check if the last client was rejected
    stdout, _ = read_process_output(overflow_client)
    success = "full" in stdout.lower() or "capacity" in stdout.lower() or "cannot join" in stdout.lower()
    
    log_test_result("Room Capacity Test", "PASSED" if success else "FAILED")
    
    # Clean up all clients
    for process in processes:
        process.stdin.write("/exit\n")
        process.stdin.flush()
        process.wait(timeout=2)


def test_max_clients():
    """Test the maximum number of concurrent clients."""
    log_test_result("Max Clients Test", "Started")
    
    # Start MAX_CLIENTS clients
    processes = []
    for i in range(1, MAX_CLIENTS + 1):
        username = f"maxtest{i}"
        process = simulate_client(username, [])
        processes.append(process)
        time.sleep(0.5)
    
    # Try to join one more client beyond the limit
    process = subprocess.Popen(
        ['./chatclient', SERVER_IP, str(SERVER_PORT)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )
    
    time.sleep(1)
    process.stdin.write("overflow\n")
    process.stdin.flush()
    
    # Wait for response
    time.sleep(3)
    stdout, _ = read_process_output(process)
    
    # Check if connection was rejected
    success = "maximum" in stdout.lower() or "limit" in stdout.lower() or "full" in stdout.lower()
    
    log_test_result("Max Clients Test", "PASSED" if success else "FAILED")
    
    # Clean up all clients
    for p in processes:
        p.stdin.write("/exit\n")
        p.stdin.flush()
        p.wait(timeout=2)
    
    process.stdin.write("/exit\n")
    process.stdin.flush()
    process.wait(timeout=2)


def test_oversized_file():
    """Test rejection of oversized files."""
    log_test_result("Oversized File Test", "Started")
    
    # Start sender and recipient
    sender = simulate_client("bigfilesender", [
        "/join fileroom",
        "Preparing to send an oversized file"
    ])
    
    time.sleep(1)
    
    recipient = simulate_client("bigfilerecipient", [
        "/join fileroom",
        "Ready to receive files"
    ])
    
    time.sleep(2)
    
    # Send an oversized file
    sender.stdin.write("/sendfile oversized.txt bigfilerecipient\n")
    sender.stdin.flush()
    
    # Wait for response
    time.sleep(5)
    
    # Check output
    stdout, _ = read_process_output(sender)
    success = "too large" in stdout.lower() or "exceeds" in stdout.lower() or "rejected" in stdout.lower()
    
    log_test_result("Oversized File Test", "PASSED" if success else "FAILED")
    
    # Clean up
    sender.stdin.write("/exit\n")
    sender.stdin.flush()
    recipient.stdin.write("/exit\n")
    recipient.stdin.flush()
    
    sender.wait(timeout=2)
    recipient.wait(timeout=2)


def test_concurrent_file_transfers():
    """Test concurrent file transfers and queue management."""
    log_test_result("Concurrent File Transfers Test", "Started")
    
    # Start one recipient and multiple senders
    recipient = simulate_client("filetarget", [
        "/join transferroom",
        "Ready to receive multiple files"
    ])
    
    time.sleep(1)
    
    # Start 6 senders (to test queue of 5 + 1 active)
    senders = []
    for i in range(1, 7):
        sender = simulate_client(f"sender{i}", [
            "/join transferroom",
            f"Sender {i} ready to send file"
        ])
        senders.append(sender)
        time.sleep(0.5)
    
    # Each sender tries to send a file to the recipient
    for i, sender in enumerate(senders, 1):
        sender.stdin.write(f"/sendfile small.txt filetarget\n")
        sender.stdin.flush()
        time.sleep(0.2)  # Small delay between sends
    
    # Wait for transfers to be processed
    time.sleep(15)
    
    # Check all senders' outputs for queue messages
    queue_messages = 0
    for i, sender in enumerate(senders, 1):
        stdout, _ = read_process_output(sender)
        if "queued" in stdout.lower() or "position" in stdout.lower():
            queue_messages += 1
    
    # At least one sender should have received a queue message
    success = queue_messages > 0
    
    log_test_result("Concurrent File Transfers Test", "PASSED" if success else "FAILED")
    
    # Clean up
    recipient.stdin.write("/exit\n")
    recipient.stdin.flush()
    for sender in senders:
        sender.stdin.write("/exit\n")
        sender.stdin.flush()
        sender.wait(timeout=2)
    recipient.wait(timeout=2)


def test_server_shutdown():
    """Test graceful server shutdown with connected clients."""
    log_test_result("Server Shutdown Test", "Started")
    
    # Start several clients
    clients = []
    for i in range(1, 4):
        client = simulate_client(f"shutdown{i}", [
            "/join shutdownroom",
            "Connected and waiting"
        ])
        clients.append(client)
        time.sleep(1)
    
    # Send SIGINT to server
    global server_process
    server_process.send_signal(signal.SIGINT)
    
    # Wait for server to process shutdown
    time.sleep(5)
    
    # Check if clients received shutdown notification
    success = True
    for i, client in enumerate(clients, 1):
        stdout, _ = read_process_output(client)
        if "shutdown" not in stdout.lower() and "disconnected" not in stdout.lower():
            success = False
            break
    
    log_test_result("Server Shutdown Test", "PASSED" if success else "FAILED")
    
    # Clean up
    for client in clients:
        client.wait(timeout=2)
    
    # Wait for server to fully shut down
    server_process.wait(timeout=5)


def read_process_output(process):
    """Read available output from a process without blocking."""
    stdout = ""
    stderr = ""
    
    if process.stdout:
        while True:
            line = process.stdout.readline()
            if not line:
                break
            stdout += line
    
    if process.stderr:
        while True:
            line = process.stderr.readline()
            if not line:
                break
            stderr += line
    
    return stdout, stderr


def log_test_result(test_name, result):
    """Log a test result."""
    with test_lock:
        if result == "Started":
            print(f"\n{BLUE}=== {test_name} ==={RESET}")
            test_results.append((test_name, "RUNNING"))
        else:
            color = GREEN if result == "PASSED" else RED
            print(f"{color}{test_name}: {result}{RESET}")
            # Update the test result
            for i, (name, _) in enumerate(test_results):
                if name == test_name:
                    test_results[i] = (name, result)
                    break


def print_summary():
    """Print a summary of all test results."""
    print(f"\n{BLUE}=== Test Summary ==={RESET}")
    passed = 0
    failed = 0
    
    for name, result in test_results:
        if result == "PASSED":
            print(f"{GREEN}{name}: {result}{RESET}")
            passed += 1
        else:
            print(f"{RED}{name}: {result}{RESET}")
            failed += 1
    
    print(f"\n{GREEN}Passed: {passed}{RESET}, {RED}Failed: {failed}{RESET}, Total: {passed + failed}")


def run_all_tests():
    """Run all test cases."""
    global server_process
    
    try:
        # Setup test environment
        setup_test_environment()
        
        # Start server
        server_process = start_server()
        if not server_process:
            print(f"{RED}Aborting tests due to server startup failure.{RESET}")
            return
        
        # Run tests
        test_basic_functionality()
        time.sleep(2)
        
        test_duplicate_username()
        time.sleep(2)
        
        test_file_transfer()
        time.sleep(2)
        
        test_room_capacity()
        time.sleep(2)
        
        test_max_clients()
        time.sleep(2)
        
        test_oversized_file()
        time.sleep(2)
        
        test_concurrent_file_transfers()
        time.sleep(2)
        
        # This test should be last as it shuts down the server
        test_server_shutdown()
        
        # Print summary
        print_summary()
        
    except Exception as e:
        print(f"{RED}Error during test execution: {e}{RESET}")
    finally:
        # Ensure server is stopped
        if server_process and server_process.poll() is None:
            stop_server(server_process)
        
        # Clean up any remaining client processes
        for process in client_processes.values():
            if process and process.poll() is None:
                process.terminate()
                process.wait()


if __name__ == "__main__":
    run_all_tests()
