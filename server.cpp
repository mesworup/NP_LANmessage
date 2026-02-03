#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define BUFFER_SIZE 1024

std::vector<SOCKET> clients;

DWORD WINAPI handleClient(LPVOID param) {
    SOCKET clientSocket = (SOCKET)param;
    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            closesocket(clientSocket);
            break;
        }

        std::string msg(buffer, bytes);

        // File transfer
        if (msg.rfind("FILE:", 0) == 0) {
            std::string filename = msg.substr(5);
            std::ofstream file(filename, std::ios::binary);

            while (true) {
                bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytes <= 0) break;

                std::string chunk(buffer, bytes);
                if (chunk.find("EOF") != std::string::npos) {
                    file.write(buffer, chunk.find("EOF"));
                    break;
                }

                file.write(buffer, bytes);
            }

            file.close();
            std::cout << "Received file: " << filename << std::endl;
        }
        else {
            // Broadcast message
            for (SOCKET c : clients) {
                if (c != clientSocket) {
                    send(c, msg.c_str(), msg.size(), 0);
                }
            }
            std::cout << msg << std::endl;
        }
    }

    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        clients.push_back(clientSocket);

        // Use Windows thread
        CreateThread(NULL, 0, handleClient, (LPVOID)clientSocket, 0, NULL);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
