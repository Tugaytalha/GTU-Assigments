#include "file_system.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <algorithm> // Added for std::find_if and std::remove_if

SuperBlock superBlock;
std::vector<int> FAT; // Simulated FAT table
std::vector<DirectoryEntry> rootDirectory; // Root directory

uint16_t getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* nowTm = std::localtime(&now);
    return (nowTm->tm_hour << 11) | (nowTm->tm_min << 5) | (nowTm->tm_sec >> 1);
}

uint16_t getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm* nowTm = std::localtime(&now);
    return ((nowTm->tm_year - 80) << 9) | ((nowTm->tm_mon + 1) << 5) | nowTm->tm_mday;
}

void saveFileSystem(const std::string& name) {
    std::ofstream ofs(name, std::ios::binary);
    ofs.write(reinterpret_cast<char*>(&superBlock), sizeof(superBlock));
    ofs.write(reinterpret_cast<char*>(FAT.data()), FAT.size() * sizeof(int));
    int dirSize = rootDirectory.size();
    ofs.write(reinterpret_cast<char*>(&dirSize), sizeof(dirSize));
    for (const auto& entry : rootDirectory) {
        ofs.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    }
    ofs.close();
}

void loadFileSystem(const std::string& name) {
    std::ifstream ifs(name, std::ios::binary);
    ifs.read(reinterpret_cast<char*>(&superBlock), sizeof(superBlock));
    FAT.resize(superBlock.totalBlocks);
    ifs.read(reinterpret_cast<char*>(FAT.data()), FAT.size() * sizeof(int));
    int dirSize;
    ifs.read(reinterpret_cast<char*>(&dirSize), sizeof(dirSize));
    rootDirectory.resize(dirSize);
    for (auto& entry : rootDirectory) {
        ifs.read(reinterpret_cast<char*>(&entry), sizeof(entry));
    }
    ifs.close();
}

void createFileSystem(const std::string& name, int blockSize) {
    superBlock.blockSize = blockSize;
    superBlock.totalBlocks = 4096; // Example for 4MB with 1KB block size
    superBlock.freeBlocks = superBlock.totalBlocks - 2; // Minus super block and root directory
    superBlock.fatStartBlock = 1;
    superBlock.rootDirStartBlock = 2;

    FAT.resize(superBlock.totalBlocks, -1);
    FAT[0] = FAT[1] = FAT[2] = 0; // Reserved blocks
    for (size_t i = 3; i < FAT.size(); ++i) {
        FAT[i] = -1; // Free blocks
    }

    rootDirectory.clear();

    saveFileSystem(name);
}

void makeDirectory(const std::string& path) {
    // Simplified for a single-level directory structure
    DirectoryEntry newDir;
    std::strncpy(newDir.name, path.c_str(), 8);
    newDir.name[7] = '\0';
    std::fill(std::begin(newDir.extension), std::end(newDir.extension), ' ');
    newDir.attributes = 0x10; // Directory attribute
    std::fill(std::begin(newDir.reserved), std::end(newDir.reserved), 0);
    newDir.time = getCurrentTime();
    newDir.date = getCurrentDate();
    newDir.firstBlock = -1;
    newDir.size = 0;

    rootDirectory.push_back(newDir);
    saveFileSystem("test_fs");
}

int allocateBlock() {
    for (size_t i = 2; i < FAT.size(); ++i) { // Skip reserved blocks
        if (FAT[i] == -1) { // Free block
            FAT[i] = 0; // Mark as used
            return i;
        }
    }
    return -1; // No free blocks
}

void freeBlocks(int startBlock) {
    int currentBlock = startBlock;
    while (currentBlock != 0) {
        int nextBlock = FAT[currentBlock];
        FAT[currentBlock] = -1;
        currentBlock = nextBlock;
    }
}

int writeFileData(const std::string& localPath, int blockSize, std::vector<int>& allocatedBlocks) {
    std::ifstream ifs(localPath, std::ios::binary);
    if (!ifs) {
        std::cerr << "Failed to open local file: " << localPath << std::endl;
        return -1;
    }

    char* buffer = new char[blockSize];
    int previousBlock = -1;
    int firstBlock = -1;

    while (ifs.read(buffer, blockSize) || ifs.gcount() > 0) {
        int block = allocateBlock();
        if (block == -1) {
            std::cerr << "No more free blocks available." << std::endl;
            delete[] buffer;
            return -1;
        }

        if (previousBlock != -1) {
            FAT[previousBlock] = block;
        } else {
            firstBlock = block;
        }
        previousBlock = block;

        std::ofstream ofs("test_fs", std::ios::binary | std::ios::in | std::ios::out);
        ofs.seekp(block * blockSize, std::ios::beg);
        ofs.write(buffer, ifs.gcount());
        ofs.close();

        allocatedBlocks.push_back(block);
    }

    delete[] buffer;
    return firstBlock;
}

void writeFile(const std::string& fsPath, const std::string& localPath) {
    std::ifstream ifs(localPath, std::ios::binary);
    if (!ifs) {
        std::cerr << "Failed to open local file: " << localPath << std::endl;
        return;
    }

    ifs.seekg(0, std::ios::end);
    std::streamsize fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    DirectoryEntry newFile;
    std::strncpy(newFile.name, fsPath.substr(0, 8).c_str(), 8);
    newFile.name[7] = '\0';
    std::strncpy(newFile.extension, fsPath.substr(9, 3).c_str(), 3);
    newFile.extension[2] = '\0';
    newFile.attributes = 0x00; // Regular file attribute
    std::fill(std::begin(newFile.reserved), std::end(newFile.reserved), 0);
    newFile.time = getCurrentTime();
    newFile.date = getCurrentDate();
    newFile.size = fileSize;

    std::vector<int> allocatedBlocks;
    newFile.firstBlock = writeFileData(localPath, superBlock.blockSize, allocatedBlocks);

    if (newFile.firstBlock == -1) {
        std::cerr << "Failed to write file data to file system." << std::endl;
        return;
    }

    rootDirectory.push_back(newFile);
    saveFileSystem("test_fs");
    std::cout << "File written successfully to " << fsPath << std::endl;
}

void readFile(const std::string& fsPath, const std::string& localPath) {
    auto it = std::find_if(rootDirectory.begin(), rootDirectory.end(), [&](const DirectoryEntry& entry) {
        return strncmp(entry.name, fsPath.substr(0, 8).c_str(), 8) == 0;
    });

    if (it == rootDirectory.end()) {
        std::cerr << "File not found: " << fsPath << std::endl;
        return;
    }

    DirectoryEntry& fileEntry = *it;
    std::ofstream ofs(localPath, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open local file: " << localPath << std::endl;
        return;
    }

    int currentBlock = fileEntry.firstBlock;
    char* buffer = new char[superBlock.blockSize];

    while (currentBlock != 0) {
        std::ifstream ifs("test_fs", std::ios::binary);
        ifs.seekg(currentBlock * superBlock.blockSize, std::ios::beg);
        ifs.read(buffer, superBlock.blockSize);
        ofs.write(buffer, ifs.gcount());
        currentBlock = FAT[currentBlock];
    }

    delete[] buffer;
    ofs.close();
    std::cout << "File read successfully from " << fsPath << " to " << localPath << std::endl;
}

void changePermissions(const std::string& path, const std::string& permissions) {
    for (auto& entry : rootDirectory) {
        if (strncmp(entry.name, path.c_str(), 8) == 0) {
            entry.attributes = 0;
            if (permissions.find('r') != std::string::npos) entry.attributes |= 0x01;
            if (permissions.find('w') != std::string::npos) entry.attributes |= 0x02;
            saveFileSystem("test_fs");
            return;
        }
    }
    std::cerr << "File not found: " << path << std::endl;
}

void addPassword(const std::string& path, const std::string& password) {
    auto it = std::find_if(rootDirectory.begin(), rootDirectory.end(), [&](const DirectoryEntry& entry) {
        return strncmp(entry.name, path.c_str(), 8) == 0;
    });

    if (it == rootDirectory.end()) {
        std::cerr << "File not found: " << path << std::endl;
        return;
    }

    DirectoryEntry& fileEntry = *it;
    std::strncpy(fileEntry.reserved, password.c_str(), 10);
    fileEntry.reserved[9] = '\0';
    saveFileSystem("test_fs");
    std::cout << "Password added to file " << path << std::endl;
}

void deleteFile(const std::string& path) {
    auto it = std::remove_if(rootDirectory.begin(), rootDirectory.end(), [&](const DirectoryEntry& entry) {
        return strncmp(entry.name, path.c_str(), 8) == 0;
    });

    if (it != rootDirectory.end()) {
        int firstBlock = it->firstBlock;
        rootDirectory.erase(it, rootDirectory.end());
        freeBlocks(firstBlock);
        saveFileSystem("test_fs");
        std::cout << "File " << path << " deleted successfully." << std::endl;
    } else {
        std::cerr << "File not found: " << path << std::endl;
    }
}

void removeDirectory(const std::string& path) {
    // Simplified to call deleteFile for now
    deleteFile(path);
}

void listDirectory(const std::string& path) {
    for (const auto& entry : rootDirectory) {
        std::cout << entry.name << "." << entry.extension << " " << entry.size << " bytes" << std::endl;
    }
}

void dumpFileSystemInfo() {
    std::cout << "Block Size: " << superBlock.blockSize << " bytes" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "FAT Start Block: " << superBlock.fatStartBlock << std::endl;
    std::cout << "Root Directory Start Block: " << superBlock.rootDirStartBlock << std::endl;

    std::cout << "\nFAT Table:" << std::endl;
    for (size_t i = 0; i < FAT.size(); ++i) {
        std::cout << i << ": " << FAT[i] << std::endl;
    }

    std::cout << "\nRoot Directory:" << std::endl;
    for (const auto& entry : rootDirectory) {
        std::cout << "Name: " << entry.name << "." << entry.extension << std::endl;
        std::cout << "Size: " << entry.size << " bytes" << std::endl;
        std::cout << "First Block: " << entry.firstBlock << std::endl;
        std::cout << "Attributes: " << static_cast<int>(entry.attributes) << std::endl;
        std::cout << "Time: " << entry.time << std::endl;
        std::cout << "Date: " << entry.date << std::endl;
        std::cout << std::endl;
    }
}
