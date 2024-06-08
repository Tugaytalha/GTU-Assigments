#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>
#include <ctime>
#include <vector>

// Constants
const int MAX_BLOCKS = 4096; // 4MB / 1KB
const int BLOCK_SIZE_1KB = 1024;
const int BLOCK_SIZE_512B = 512;
const int FILE_SYSTEM_SIZE = 4 * 1024 * 1024; // 4 MB

// Directory Entry Structure
struct DirectoryEntry {
    std::string fileName;
    int size;
    bool readPermission;
    bool writePermission;
    std::time_t lastModification;
    std::time_t creationTime;
    std::string password;
};

// Function Declarations
void createFileSystem(const std::string& fileName, int blockSize);
void listDirectory(const std::string& path);
void makeDirectory(const std::string& path);
void removeDirectory(const std::string& path);
void dumpFileSystemInfo();
void writeFile(const std::string& path, const std::string& linuxFile);
void readFile(const std::string& path, const std::string& linuxFile);
void deleteFile(const std::string& path);
void changePermissions(const std::string& path, const std::string& permissions);
void addPassword(const std::string& path, const std::string& password);

#endif // FILE_SYSTEM_H
