#include <iostream>
#include <cstring>
#include <map>
#include <winsock2.h>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 479;
const size_t CACHE_SIZE = 1024 * 1024; // 1MB

std::map<std::string, std::string> cache;

const std::string FILENAME = "cache_data.txt";

//void printCache() {
   // std::cout << "\x1B[2J\x1B[H";
    //std::cout << "Current Cache Contents:" << std::endl;
   // std::cout << "-----------------------" << std::endl;
  //  for (const auto& entry : cache) {
  //      std::cout << "Key: " << entry.first << ", Value: " << entry.second << std::endl;
  //  }
  //  std::cout << "-----------------------" << std::endl;
//}

void saveCacheToFile() {
    std::ofstream outFile(FILENAME);
    if (outFile.is_open()) {
        for (const auto& entry : cache) {
            outFile << entry.first << ":" << entry.second << std::endl;
        }
        outFile.close();
    }
}

void loadCacheFromFile() {
    std::ifstream inFile(FILENAME);
    if (inFile.is_open()) {
        cache.clear();
        std::string line;
        while (std::getline(inFile, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                cache[key] = value;
            }
        }
        inFile.close();
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    loadCacheFromFile();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Error listening: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Cache service started on port " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting client connection: " << WSAGetLastError() << std::endl;
            continue;
        }

        char request[256];
        memset(request, 0, sizeof(request));
        recv(clientSocket, request, sizeof(request), 0);

        std::string response;
        if (strncmp(request, "GET", 3) == 0) {
            std::string key = request + 4;
            if (cache.find(key) != cache.end()) {
                response = cache[key];

                send(clientSocket, response.c_str(), response.size(), 0);
            }
            else {
                response = "Key not found in cache";

                send(clientSocket, response.c_str(), response.size(), 0);
            }
            // No need to close client socket here
        }
        else if (strncmp(request, "REWRITE", 7) == 0) {
            std::string data = request + 8;
            size_t pos = data.find(':');
            if (pos != std::string::npos) {
                std::string key = data.substr(0, pos);
                std::string value = data.substr(pos + 1);
                if (cache.find(key) != cache.end()) {
                    cache[key] = value;
                    response = "Key " + key + " rewritten with value " + value;
                    //printCache();
                    saveCacheToFile();
                }
                else {
                    response = "Key not found in cache";
                }
            }
            else {
                response = "Invalid data format";
            }
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else if (strncmp(request, "SET", 3) == 0) {
            std::string data = request + 4;
            size_t pos = data.find(':');
            if (pos != std::string::npos) {
                std::string key = data.substr(0, pos);
                std::string value = data.substr(pos + 1);
                cache[key] = value;
                response = key + ":" + value + " pair stored in cache";
                //printCache();
                saveCacheToFile();
            }
            else {
                response = "Invalid data format";
            }
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else if (strncmp(request, "PRINT", 5) == 0) {
            //printCache();
            response = "Printed cache contents";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else {
            response = "Invalid command";
            send(clientSocket, response.c_str(), response.size(), 0);
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
