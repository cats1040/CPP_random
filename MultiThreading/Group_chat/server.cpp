#include <iostream>
#include <winsock2.h> // Windows socket API(networking)
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::vector<SOCKET> clients;
std::mutex clients_mutex;
/*
    clients_mutex ensures that multiple threads
    do not modify clients at the same time.
*/

void broadcast_message(const std::string &message, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients)
    {
        if (client != sender)
        {
            std::string formatted_msg = "User" + std::to_string(sender) + ": " + message + "\n\n";
            send(client, formatted_msg.c_str(), formatted_msg.size(), 0);
        }
    }
}

void handle_client(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }

    std::cout << "User" << client_socket << " joined the chat. Total users: " << clients.size() << std::endl;
    std::string welcome_msg = "Server: Welcome to the chat! Users online: " + std::to_string(clients.size()) + "\n\n";
    send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);

    //  Infinite loop to keep receiving messages until the client disconnects
    while (true)
    {
        std::string message;
        bool message_complete = false;

        // Receive message
        while (!message_complete)
        {
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_received > 0)
            {
                message.append(buffer, bytes_received);

                // Check for message delimiter
                size_t delimiter_pos = message.find("\n\n");
                if (delimiter_pos != std::string::npos)
                {
                    message = message.substr(0, delimiter_pos);
                    message_complete = true;
                }
            }
            else if (bytes_received == 0)
            {
                std::cout << "User" << client_socket << " left the chat\n";
                goto disconnect;
            }
            else
            {
                std::cerr << "recv error: " << WSAGetLastError() << std::endl;
                goto disconnect;
            }
        }

        std::cout << "Message from User" << client_socket << ": " << message << std::endl;

        // Broadcast to all clients
        if (!message.empty())
        {
            broadcast_message(message, client_socket);
        }
    }

// This label is used by the goto statement to jump to the disconnection logic.
disconnect:
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
}
    closesocket(client_socket);
}

int main()
{
    WSADATA wsaData;
    SOCKET server_socket;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    /*
        _in stands for internet
        AF_INET: Address Family Internet (IPv4)
        For IPv6, AF_INET6 is used

        INADDR_ANY: This binds the server to all
                    available network interfaces
                    (localhost, external network IPs, etc.).

                    Common for servers because they might accept
                    connections from multiple networks.

                    The system will automatically choose the most suitable IP.
    */

    // Bind socket
    // bind() assigns the socket to PORT.
    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)))
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(server_socket, SOMAXCONN))
    {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Chat Server started on port " << PORT << std::endl;

    while (true)
    {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}