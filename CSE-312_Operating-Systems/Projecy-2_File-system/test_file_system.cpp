#include "file_system.h"
#include <iostream>
#include <fstream>
#include <cassert>

void runTests() {
    // Create a new file system
    createFileSystem("test_fs", 1024);

    // Make a new directory
    makeDirectory("testdir");

    // Write a file to the file system
    std::ofstream tempFile("tempfile.txt");
    tempFile << "This is a test file.";
    tempFile.close();
    writeFile("testdir/testfile.txt", "tempfile.txt");

    // List directory contents
    std::cout << "\nDirectory listing for /testdir:" << std::endl;
    listDirectory("/testdir");

    // Change file permissions
    changePermissions("testdir/testfile.txt", "rw");
    std::cout << "\nPermissions after change:" << std::endl;
    listDirectory("/testdir");

    // Add a password to the file
    addPassword("testdir/testfile.txt", "password123");
    std::cout << "\nPassword added to file:" << std::endl;
    listDirectory("/testdir");

    // Read the file from the file system
    readFile("testdir/testfile.txt", "readfile.txt");
    std::ifstream readFile("readfile.txt");
    std::string content((std::istreambuf_iterator<char>(readFile)), std::istreambuf_iterator<char>());
    readFile.close();
    
    // Debug print
    std::cout << "\nContent read from file: " << content << std::endl;

    // Verify content
    assert(content == "This is a test file.");

    // Delete the file
    deleteFile("testdir/testfile.txt");
    std::cout << "\nDirectory listing after file deletion:" << std::endl;
    listDirectory("/testdir");

    // Remove the directory
    removeDirectory("testdir");
    std::cout << "\nDirectory listing after directory removal:" << std::endl;
    listDirectory("/");

    // Dump file system information
    std::cout << "\nFile system info:" << std::endl;
    dumpFileSystemInfo();

    // Clean up temporary files
    std::remove("tempfile.txt");
    std::remove("readfile.txt");
}

int main() {
    std::cout << "Running file system tests..." << std::endl;
    runTests();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
