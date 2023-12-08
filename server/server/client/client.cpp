#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include<thread>
#include <random>
#define N 1000
#pragma comment(lib, "ws2_32")

#define H_ECHO 8282
#define H_COORDINATE 3142

void SendCoordinates(SOCKET clientSocket) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(-1, 1);  // -1, 0, 1 중 하나를 랜덤으로 선택

    int currentX = 0;
    int currentY = 0;

    while (true) {
        int deltaX = dis(gen);
        int deltaY = dis(gen);

        int newX = currentX + deltaX;
        int newY = currentY + deltaY;

        if (newX != currentX || newY != currentY) {
            currentX = newX;
            currentY = newY;

            std::string coordinate = std::to_string(currentX) + "," + std::to_string(currentY);
            //std::cout << x << "," << y << std::endl;
            int number = H_COORDINATE;
            int packetLength = 2 * sizeof(int) + coordinate.length();
            char* packetBuffer = new char[packetLength];

            memcpy(packetBuffer, &packetLength, sizeof(int));
            memcpy(packetBuffer + sizeof(int), &number, sizeof(int));
            memcpy(packetBuffer + 2 * sizeof(int), coordinate.c_str(), coordinate.length());
            send(clientSocket, packetBuffer, packetLength, 0);

            delete[] packetBuffer;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void ClientThread() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup() failed." << std::endl;
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "socket() failed." << std::endl;
        WSACleanup();
        return;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    double x = 0;
    double y = 0;
    std::string message;
    char buffer[1024];
    std::thread coordinateThread(SendCoordinates, clientSocket);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    /* Echo func
    while (true) {

        std::cout << "Enter a message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        int number = H_ECHO;
        int packetLength = sizeof(int) + message.length();
        char* packetBuffer = new char[packetLength];

        memcpy(packetBuffer, &number, sizeof(int));
        memcpy(packetBuffer + sizeof(int), message.c_str(), message.length());
        send(clientSocket, packetBuffer, packetLength, 0);

        // 서버로부터 응답 수신
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        int packetNumber;
        memcpy(&packetNumber, buffer, sizeof(int));
        int messageLength = sizeof(buffer) - sizeof(int);
        char* messageData = buffer + sizeof(int);
        if (messageLength > 0) {
            buffer[messageLength] = '\0';
            std::cout << "Server response: " << messageData << std::endl;
        }
        else if (messageData == 0) {
            std::cerr << "Connection closed by server." << std::endl;
            break;
        }
        else {
            std::cerr << "recv() failed." << std::endl;
            break;
        }

        delete[] packetBuffer;
    }
    */
    coordinateThread.join();

    closesocket(clientSocket);
    WSACleanup();
}

int main() {

    std::thread clientThreads[N];

    for (int i = 0; i < N; ++i)
    {
        clientThreads[i] = std::thread(ClientThread);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    for (int i = 0; i < N; ++i)
        clientThreads[i].join();

    return 0;
}