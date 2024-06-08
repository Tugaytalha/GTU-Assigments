#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>
#include <vector>
#include <ctime>

#pragma pack(push, 1)
struct DirectoryEntry {
    char name[8];
    char extension[3];
    uint8_t attributes;
    char reserved[10];
    uint16_t time;
    uint16_t date;
    uint16_t firstBlock;
    uint32_t size;
};
#pragma pack(pop)

struct SuperBlock {
    int blockSize;
    int totalBlocks;
    int freeBlocks;
    int fatStartBlock;
    int rootDirStartBlock;
};

void createFileSystem(const std::string& name, int blockSize);
void makeDirectory(const std::string& path);
void writeFile(const std::string& fsPath, const std::string& localPath);
void listDirectory(const std::string& path);
void changePermissions(const std::string& path, const std::string& permissions);
void addPassword(const std::string& path, const std::string& password);
void readFile(const std::string& fsPath, const std::string& localPath);
void deleteFile(const std::string& path);
void removeDirectory(const std::string& path);
void dumpFileSystemInfo();
void loadFileSystem(const std::string& name);
void saveFileSystem(const std::string& name);

#endif // FILE_SYSTEM_H
