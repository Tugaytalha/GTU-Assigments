#!/bin/bash

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

echo "2.a Creating file 'testDir/example.txt'..."
./fileManager createFile "testDir/example.txt"
echo

echo "2.b. Appending 'Hello, World!' to file..."
./fileManager appendToFile "testDir/example.txt" "Hello, World!"
echo

echo "3. Listing files in 'testDir'..."
./fileManager listDir "testDir"
echo

echo "4.a. Reading file after append..."
./fileManager readFile "testDir/example.txt"
echo

echo "4.b. Appending 'New Line' to file..."
./fileManager appendToFile "testDir/example.txt" "New Line"
echo

echo "5.a. Deleting file 'testDir/example.txt'..."
./fileManager deleteFile "testDir/example.txt"
echo

echo "5.b. Showing logs..."
./fileManager showLogs
echo

echo "=============================="
echo "TESTING COMPLETED"
echo "==============================" 