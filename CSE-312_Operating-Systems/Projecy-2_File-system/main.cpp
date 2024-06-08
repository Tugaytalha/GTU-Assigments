#include "file_system.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file_system> <operation> [parameters]" << std::endl;
        return 1;
    }

    std::string fileSystem = argv[1];
    std::string operation = argv[2];

    if (operation == "dir") {
        std::string path = argv[3];
        listDirectory(path);
    } else if (operation == "mkdir") {
        std::string path = argv[3];
        makeDirectory(path);
    } else if (operation == "rmdir") {
        std::string path = argv[3];
        removeDirectory(path);
    } else if (operation == "dumpe2fs") {
        dumpFileSystemInfo();
    } else if (operation == "write") {
        std::string path = argv[3];
        std::string linuxFile = argv[4];
        writeFile(path, linuxFile);
    } else if (operation == "read") {
        std::string path = argv[3];
        std::string linuxFile = argv[4];
        readFile(path, linuxFile);
    } else if (operation == "del") {
        std::string path = argv[3];
        deleteFile(path);
    } else if (operation == "chmod") {
        std::string path = argv[3];
        std::string permissions = argv[4];
        changePermissions(path, permissions);
    } else if (operation == "addpw") {
        std::string path = argv[3];
        std::string password = argv[4];
        addPassword(path, password);
    } else if (operation == "mkfs") {
        std::string name = argv[3];
        int blockSize = std::stoi(argv[4]);
        createFileSystem(name, blockSize);
    } else if (operation == "help") {
        std::cout << "Operations:" << std::endl;
        std::cout << "dir <path> - List directory contents" << std::endl;
        std::cout << "mkdir <path> - Make a new directory" << std::endl;
        std::cout << "rmdir <path> - Remove a directory" << std::endl;
        std::cout << "dumpe2fs - Dump file system information" << std::endl;
        std::cout << "write <path> <file> - Write a file to the file system" << std::endl;
        std::cout << "read <path> <file> - Read a file from the file system" << std::endl;
        std::cout << "del <path> - Delete a file" << std::endl;
        std::cout << "chmod <path> <permissions> - Change file permissions" << std::endl;
        std::cout << "addpw <path> <password> - Add a password to a file" << std::endl;
        std::cout << "mkfs <name> <blockSize> - Create a new file system" << std::endl;
    } else if (operation == "loadfs") {
        std::string name = argv[3];
        loadFileSystem(name);
    } else if (operation == "savefs") {
        std::string name = argv[3];
        saveFileSystem(name);
    }
    else {
        std::cerr << "Unknown operation: " << operation << std::endl;
    }

    return 0;
}
