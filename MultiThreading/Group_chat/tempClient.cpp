#include <iostream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const char *SERVER_IP = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main()
{
    WSADATA wsaData;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    /*
        Winsock is a networking API for Windows
        that allows programs to communicate over a
        network using sockets.
    */
    // ---- Initialize Winsock (first step, before using its functions)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    /*
     std::cout << "Winsock Initialized Successfully!" << std::endl;
     std::cout << "Description: " << wsaData.szDescription << std::endl;
     std::cout << "System Status: " << wsaData.szSystemStatus << std::endl;
     std::cout << "Winsock Version: " << LOBYTE(wsaData.wVersion) << "."
     << HIBYTE(wsaData.wVersion) << std::endl;
    */

    /*
        socket(int domain, int type, int protocol);

        domain: Specifies type of address family (type of IP address)
            here, AF_INET -> IPv4
        type: Specifies socket type (TCP or UDP)
            here, SOCK_STREAM -> TCP socket
            SOCK_DGRAM -> UDP socket
            SOCK_RAW -> Allows raw access to network protocols like
                        ICMP or custom protocols
        0 (third param): 0 means it will choose the default protocol for
                         the specified socket type
                         SOCK_STREAM -> TCP
                         SOCK_DGRAM -> UDP

    */
    // ---- Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    /*
        sin_port stores the port number
        to which the server will bind or the client
        will connect
        htons() -> Host to Network Short, convert the port number
                   from host byte order to network byte order

        Host Byte Order → The way numbers are stored in your machine
                          (could be Little-Endian or Big-Endian).

        Network Byte Order → Standardized Big-Endian (most significant byte first).

        Example:
        Port 8080 in hexadecimal is 1F90.
        On a Little-Endian system, it would be stored as 90 1F.
        On a Big-Endian (Network Byte Order) system, it would be stored as 1F 90.

        htons() ensures the port is always in Big-Endian for consistent communication.
    */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /*
        inet_pton() converts a human readable IPv4 or IPv6
        address to a bunary form that is used by the network
        (in_addr struct for IPv4)

        int inet_pton(int af, const char *src, void *dst);

        dst -> Pointer to a structure (&server_addr.sin_addr)
               where the converted binary form will be stored.
    */
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // ---- Connect to server
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)))
    {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;

        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server. Type 'exit' to quit.\n";

    while (true)
    {
        // Get user input
        std::string message;
        std::cout << "You : ";
        std::getline(std::cin, message);

        // Exit
        if (message == "exit")
        {
            break;
        }

        // Send message
        if (send(client_socket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            break;
        }

        // Receive response
        char buffer[BUFFER_SIZE];
        int bytes_received;
        std::string response;

        while (true)
        {
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_received > 0)
            {
                response.append(buffer, bytes_received);
                // Check for end of message indicator (you can define your own protocol)
                if (response.find("\n\n") != std::string::npos)
                {
                    response.erase(response.find("\n\n"), 2);
                    break;
                }
            }
            else if (bytes_received == 0)
            {
                std::cout << "Server closed connection\n";
                closesocket(client_socket);
                WSACleanup();
                return 0;
            }
            else
            {
                std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
                break;
            }
        }

        std::cout << "Server: " << response << std::endl;
    }

    // ---- Cleanup
    closesocket(client_socket);
    WSACleanup();
    return 0;
}