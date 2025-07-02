#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")
// Tells the compiler to link the specified library automatically.

const char *SERVER_IP = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::atomic<bool> running(true);

void receiver(SOCKET sock)
{
    char buffer[BUFFER_SIZE];
    while (running)
    {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        /*
            recv() → Receives data from the server using the socket sock.
            BUFFER_SIZE - 1 → Leaves space for the null terminator ('\0').
        */
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            std::cout << "\r" << buffer << "\nYou: " << std::flush;
            /*
                \r → Carriage return, moves the cursor to
                the beginning of the line.

                buffer[bytes_received] = '\0'; → Null terminates the buffer to
                convert it into a valid C-style string.

                std::flush → Forces the output to display
                immediately without buffering.
            */
        }
        else if (bytes_received == 0)
        {
            std::cout << "\nServer disconnected\n";
            running = false;
            break;
        }
        else
        {
            std::cerr << "\nReceive error: " << WSAGetLastError() << std::endl;
            running = false;
            break;
        }
    }
}

int main()
{
    WSADATA wsaData;
    // Structure that contains details about the Winsock implementation.
    SOCKET sock;
    sockaddr_in server_addr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    /*
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    */

    // connecting to server
    connect(sock, (sockaddr *)&server_addr, sizeof(server_addr));
    /*
        connect() → Establishes a connection to the server.
        sockaddr * → Typecasting to sockaddr, as connect() requires it.
        sizeof() → Provides the size of server_addr.
    */

    std::thread(receiver, sock).detach();

    while (running)
    {
        std::string msg;
        std::cout << "You: ";
        std::getline(std::cin, msg);

        if (msg == "exit")
        {
            running = false;
            break;
        }

        msg += "\n\n";
        send(sock, msg.c_str(), msg.size(), 0);
        /*
            send() → Sends the message to the server using the socket.
            msg.c_str() → Converts the string to a
                          C-style string for compatibility.
        */
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

// g++ client.cpp -o client.exe -lws2_32