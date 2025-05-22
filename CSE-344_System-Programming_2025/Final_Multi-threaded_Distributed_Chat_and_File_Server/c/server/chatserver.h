#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <ctime>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <memory>
#include <csignal>
#include <regex>

// For cross-platform socket programming
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define SOCKET_ERROR_VAL INVALID_SOCKET
    #define close_socket(s) closesocket(s)
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <semaphore.h>
    typedef int socket_t;
    #define SOCKET_ERROR_VAL (-1)
    #define close_socket(s) close(s)
#endif

// Constants
const int MAX_CLIENTS = 15;
const int MAX_ROOM_CAPACITY = 15;
const int MAX_UPLOAD_QUEUE = 5;
const int MAX_USERNAME_LENGTH = 16;
const int MAX_ROOM_NAME_LENGTH = 32;
const size_t MAX_FILE_SIZE = 3 * 1024 * 1024; // 3MB
const std::string ALLOWED_FILE_EXTENSIONS[] = {".txt", ".pdf", ".jpg", ".png"};

// Message types
enum class MessageType {
    SYSTEM,
    BROADCAST,
    WHISPER,
    FILE_TRANSFER,
    ERROR
};

// File transfer status
enum class FileTransferStatus {
    PENDING,
    IN_PROGRESS,
    COMPLETED,
    FAILED
};

// File transfer request
struct FileTransferRequest {
    std::string sender;
    std::string receiver;
    std::string filename;
    size_t filesize;
    std::chrono::system_clock::time_point requestTime;
    FileTransferStatus status;
    
    FileTransferRequest(const std::string& s, const std::string& r, const std::string& f, size_t fs)
        : sender(s), receiver(r), filename(f), filesize(fs),
          requestTime(std::chrono::system_clock::now()), status(FileTransferStatus::PENDING) {}
};

// Client structure
struct Client {
    socket_t socket;
    std::string username;
    std::string currentRoom;
    std::string ipAddress;
    bool isActive;
    
    Client(socket_t s, const std::string& user, const std::string& ip)
        : socket(s), username(user), ipAddress(ip), isActive(true) {}
};

// Room structure
struct Room {
    std::string name;
    std::set<std::string> members;
    
    explicit Room(const std::string& n) : name(n) {}
};

// Chat server class
class ChatServer {
public:
    ChatServer(int port);
    ~ChatServer();
    
    void start();
    void stop();
    
private:
    // Server socket and port
    socket_t serverSocket;
    int serverPort;
    
    // Synchronization primitives
    std::mutex clientsMutex;
    std::mutex roomsMutex;
    std::mutex logMutex;
    std::mutex fileQueueMutex;
    std::condition_variable fileQueueCV;
    
    // Semaphore for file upload queue
    #ifdef _WIN32
        std::counting_semaphore<MAX_UPLOAD_QUEUE> fileQueueSemaphore;
    #else
        sem_t fileQueueSemaphore;
    #endif
    
    // Clients, rooms, and file queue
    std::map<std::string, std::shared_ptr<Client>> clients;
    std::map<std::string, std::shared_ptr<Room>> rooms;
    std::queue<std::shared_ptr<FileTransferRequest>> fileTransferQueue;
    
    // Server state
    std::atomic<bool> running;
    std::ofstream logFile;
    
    // Thread pool
    std::vector<std::thread> clientThreads;
    std::thread fileQueueThread;
    
    // Signal handling
    static ChatServer* instance;
    static void signalHandler(int signal);
    
    // Helper methods
    void acceptConnections();
    void handleClient(std::shared_ptr<Client> client);
    void processCommand(std::shared_ptr<Client> client, const std::string& command);
    void processFileQueue();
    
    // Command handlers
    void handleJoin(std::shared_ptr<Client> client, const std::string& roomName);
    void handleLeave(std::shared_ptr<Client> client);
    void handleBroadcast(std::shared_ptr<Client> client, const std::string& message);
    void handleWhisper(std::shared_ptr<Client> client, const std::string& targetUser, const std::string& message);
    void handleSendFile(std::shared_ptr<Client> client, const std::string& filename, const std::string& targetUser);
    void handleExit(std::shared_ptr<Client> client);
    
    // Utility methods
    void sendMessage(std::shared_ptr<Client> client, const std::string& message, MessageType type);
    void sendMessageToRoom(const std::string& roomName, const std::string& sender, const std::string& message);
    void log(const std::string& message);
    bool isValidUsername(const std::string& username);
    bool isValidRoomName(const std::string& roomName);
    bool isAllowedFileExtension(const std::string& filename);
    std::string getCurrentTimestamp();
    void cleanupClient(std::shared_ptr<Client> client);
    void transferFile(std::shared_ptr<FileTransferRequest> request);
};

#endif // CHATSERVER_H
