#include "file_system.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <ctime>

// Helper functions
std::vector<DirectoryEntry> directoryEntries; // This would ideally be read from the file system

void createFileSystem(const std::string& fileName, int blockSize) {
    std::ofstream fileSystem(fileName, std::ios::binary);
    if (!fileSystem) {
        std::cerr << "Error creating file system" << std::endl;
        return;
    }

    // Initialize the file system with empty data
    std::vector<char> emptyData(FILE_SYSTEM_SIZE, 0);
    fileSystem.write(emptyData.data(), emptyData.size());
    fileSystem.close();
    std::cout << "File system created with block size: " << blockSize << " KB" << std::endl;
}

void listDirectory(const std::string& path) {
    // For simplicity, assuming root directory and printing all entries
    for (const auto& entry : directoryEntries) {
        std::cout << "File: " << entry.fileName << " Size: " << entry.size << " Permissions: "
                  << (entry.readPermission ? "R" : "") << (entry.writePermission ? "W" : "") << std::endl;
    }
}

void makeDirectory(const std::string& path) {
    // Simply adding to the directory entries for demonstration
    DirectoryEntry newDir;
    newDir.fileName = path;
    newDir.size = 0;
    newDir.readPermission = true;
    newDir.writePermission = true;
    newDir.creationTime = std::time(nullptr);
    newDir.lastModification = std::time(nullptr);
    directoryEntries.push_back(newDir);
    std::cout << "Directory created: " << path << std::endl;
}

void removeDirectory(const std::string& path) {
    // For simplicity, just removing the first match
    directoryEntries.erase(
        std::remove_if(directoryEntries.begin(), directoryEntries.end(), [&](const DirectoryEntry& entry) {
            return entry.fileName == path;
        }), directoryEntries.end());
    std::cout << "Directory removed: " << path << std::endl;
}

void dumpFileSystemInfo() {
    std::cout << "Block count: " << MAX_BLOCKS << std::endl;
    std::cout << "Free blocks: " << MAX_BLOCKS - directoryEntries.size() << std::endl;
    std::cout << "Number of files and directories: " << directoryEntries.size() << std::endl;
    for (const auto& entry : directoryEntries) {
        std::cout << "File: " << entry.fileName << " Size: " << entry.size << std::endl;
    }
}

void writeFile(const std::string& path, const std::string& linuxFile) {
    // Create a new directory entry
    DirectoryEntry newFile;
    newFile.fileName = path;
    newFile.readPermission = true;
    newFile.writePermission = true;
    newFile.creationTime = std::time(nullptr);
    newFile.lastModification = std::time(nullptr);

    // Read data from linux file and write to file system
    std::ifstream inputFile(linuxFile, std::ios::binary);
    if (inputFile) {
        inputFile.seekg(0, std::ios::end);
        newFile.size = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        directoryEntries.push_back(newFile);
        inputFile.close();
        std::cout << "File written: " << path << " Size: " << newFile.size << std::endl;
    } else {
        std::cerr << "Error opening input file" << std::endl;
    }
}

void readFile(const std::string& path, const std::string& linuxFile) {
    for (const auto& entry : directoryEntries) {
        if (entry.fileName == path) {
            std::ofstream outputFile(linuxFile, std::ios::binary);
            if (outputFile) {
                // Simulate reading the file data (this should be actual file data in a real FS)
                std::vector<char> fileData(entry.size, '0');
                outputFile.write(fileData.data(), fileData.size());
                outputFile.close();
                std::cout << "File read: " << path << " Size: " << entry.size << std::endl;
            } else {
                std::cerr << "Error opening output file" << std::endl;
            }
            return;
        }
    }
    std::cerr << "File not found: " << path << std::endl;
}

void deleteFile(const std::string& path) {
    directoryEntries.erase(
        std::remove_if(directoryEntries.begin(), directoryEntries.end(), [&](const DirectoryEntry& entry) {
            return entry.fileName == path;
        }), directoryEntries.end());
    std::cout << "File deleted: " << path << std::endl;
}

void changePermissions(const std::string& path, const std::string& permissions) {
    for (auto& entry : directoryEntries) {
        if (entry.fileName == path) {
            entry.readPermission = permissions.find('r') != std::string::npos;
            entry.writePermission = permissions.find('w') != std::string::npos;
            std::cout << "Permissions changed for file: " << path << std::endl;
            return;
        }
    }
    std::cerr << "File not found: " << path << std::endl;
}

void addPassword(const std::string& path, const std::string& password) {
    for (auto& entry : directoryEntries) {
        if (entry.fileName == path) {
            entry.password = password;
            std::cout << "Password added for file: " << path << std::endl;
            return;
        }
    }
    std::cerr << "File not found: " << path << std::endl;
}
