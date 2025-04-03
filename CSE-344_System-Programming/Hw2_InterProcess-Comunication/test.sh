#!/bin/bash

# Test script for IPC Daemon program

echo "Starting tests for IPC Daemon program..."

# Compile the program
echo "Compiling program..."
make clean
make

if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting tests."
    exit 1
fi

echo "Compilation successful."

# Test 1: Basic functionality
echo -e "\nTest 1: Basic functionality with two positive integers"
echo "Running: ./ipc_daemon 42 17"
./ipc_daemon 42 17

sleep 2
echo "Expected result: The larger number is 42"

# Test 2: Negative numbers
echo -e "\nTest 2: Testing with negative numbers"
echo "Running: ./ipc_daemon -10 -5"
./ipc_daemon -10 -5

sleep 2
echo "Expected result: The larger number is -5"

# Test 3: Equal numbers
echo -e "\nTest 3: Testing with equal numbers"
echo "Running: ./ipc_daemon 100 100"
./ipc_daemon 100 100

sleep 2
echo "Expected result: Both numbers are equal (100)"

# Test 4: Invalid input (not enough arguments)
echo -e "\nTest 4: Testing with invalid input (not enough arguments)"
echo "Running: ./ipc_daemon 42"
./ipc_daemon 42 2>/dev/null

if [ $? -ne 0 ]; then
    echo "Test passed: Program correctly detected missing argument"
else
    echo "Test failed: Program should have detected missing argument"
fi

# Test 5: Check for zombie processes
echo -e "\nTest 5: Checking for zombie processes"
echo "Running: ./ipc_daemon 50 25"
./ipc_daemon 50 25 &
PARENT_PID=$!

sleep 25
echo "Checking for zombie processes..."
zombies=$(ps aux | grep defunct | grep -v grep | wc -l)

if [ $zombies -eq 0 ]; then
    echo "Test passed: No zombie processes detected"
else
    echo "Test failed: Detected $zombies zombie processes"
fi

kill -9 $PARENT_PID 2>/dev/null

# Test 6: Check log file
echo -e "\nTest 6: Checking daemon log file"
if [ -f "daemon.log" ]; then
    echo "Test passed: Daemon log file exists"
    echo "Log file contents:"
    echo "-------------------"
    tail -n 10 daemon.log
    echo "-------------------"
else
    echo "Test failed: Daemon log file not found"
fi

# Cleanup
echo -e "\nCleaning up..."
make clean

echo "All tests completed." 