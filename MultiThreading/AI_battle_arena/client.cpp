#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const char *SERVER_IP = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main()
{
    WSADATA wsaData;
    SOCKET client_socket;
    sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to server
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Get user input
    std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);

    // Send message
    int bytes_sent = send(client_socket, message.c_str(), message.size(), 0);
    if (bytes_sent == SOCKET_ERROR)
    {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
    }
    else
    {
        // Shutdown sending side to signal end of request
        shutdown(client_socket, SD_SEND);
    }

    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes_received;
    std::string response;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        response.append(buffer, bytes_received);
    }

    if (!response.empty())
    {
        std::cout << "Server response: " << response << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}