// Remove #include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define BUFFER_SIZE 1024

DWORD WINAPI receiveMessagesThread(LPVOID param) {
    SOCKET sock = (SOCKET)param;
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;
        std::cout << std::string(buffer, bytes) << std::endl;
    }
    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        std::cerr << "Connection failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    CreateThread(NULL, 0, receiveMessagesThread, (LPVOID)sock, 0, NULL);

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        if (input.rfind("/file ", 0) == 0) {
            std::string filename = input.substr(6);
            std::ifstream file(filename, std::ios::binary);

            if (!file) {
                std::cout << "File not found\n";
                continue;
            }

            std::string header = "FILE:" + filename;
            send(sock, header.c_str(), header.size(), 0);

            char buffer[BUFFER_SIZE];
            while (file.read(buffer, BUFFER_SIZE)) {
                send(sock, buffer, BUFFER_SIZE, 0);
            }
            send(sock, buffer, file.gcount(), 0);
            send(sock, "EOF", 3, 0);
            file.close();
        } else {
            std::string msg = "MSG:" + input;
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
