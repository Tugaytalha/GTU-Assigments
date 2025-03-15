#!/bin/bash

# Test script for fileManager program
# This script follows the testing scenario specified in the requirements

echo "=============================="
echo "TESTING FILE MANAGER PROGRAM"
echo "=============================="
echo

# Ensure the program is compiled
make clean && make
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

echo "1. Creating directory 'testDir'..."
./fileManager createDir "testDir"
echo

echo "2. Creating file 'testDir/example.txt'..."
./fileManager createFile "testDir/example.txt"
echo

echo "3. Listing files in 'testDir'..."
./fileManager listDir "testDir"
echo

echo "4. Reading file 'testDir/example.txt'..."
./fileManager readFile "testDir/example.txt"
echo

echo "5. Appending 'Hello, World!' to file..."
./fileManager appendToFile "testDir/example.txt" "Hello, World!"
echo

echo "6. Reading file after append..."
./fileManager readFile "testDir/example.txt"
echo

echo "7. Appending 'New Line' to file..."
./fileManager appendToFile "testDir/example.txt" "New Line"
echo

echo "8. Reading file after second append..."
./fileManager readFile "testDir/example.txt"
echo

echo "9. Deleting file 'testDir/example.txt'..."
./fileManager deleteFile "testDir/example.txt"
echo

echo "10. Showing logs..."
./fileManager showLogs
echo

echo "11. Deleting directory 'testDir'..."
./fileManager deleteDir "testDir"
echo

echo "=============================="
echo "TESTING COMPLETED"
echo "==============================" 