#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>

/*
    it ensures linker knows where to find the functions
    ( socket(), bind(), listen(), accept(), recv(), send() )
    during the build process
*/
#pragma comment(lib, "ws2_32.lib")

bool ends_with(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
    {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

const int PORT = 8080;
const int BUFFER_SIZE = 1024; // in bytes
/*
    A buffer is a temporary storage area used to hold
    data while it is being transferred between 2 locations,
    typically between:
    - Memory and a file
    - Memmory and a network socket
    - Input and output devices

    1024 bytes = 1 KB
*/

std::string read_file(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found.";
    }

    /*
        A stringstream associates a string object
        with a stream allowing you to read from the
        string as if it were a stream (like cin)
    */
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    std::string content_type = "text/html";

    if (ends_with(filepath, ".js"))
    {
        content_type = "application/javascript";
    }
    else if (ends_with(filepath, ".css"))
    {
        content_type = "text/css";
    }

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\nContent-Type: " << content_type
             << "\r\nContent-Length: " << content.size()
             << "\r\n\r\n"
             << content;
    return response.str();
}

void handle_client(SOCKET client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received <= 0)
    {
        closesocket(client_socket);
        return;
    }

    std::string request(buffer, bytes_received);
    std::cout << "[SERVER] Received request:\n"
              << request << std::endl;

    std::string response;
    if (request.find("GET / ") != std::string::npos)
    {
        response = read_file("index.html");
    }
    else if (request.find("POST /submit") != std::string::npos)
    {
        std::cout << "[SERVER] Handling POST /submit" << std::endl;

        // Extract body data (after the last \r\n\r\n)
        size_t body_pos = request.find("\r\n\r\n");
        if (body_pos != std::string::npos)
        {
            std::string body = request.substr(body_pos + 4);

            // Extract prompt data from body
            size_t pos = body.find("prompt=");
            if (pos != std::string::npos)
            {
                std::string prompt = body.substr(pos + 7);
                prompt = prompt.empty() ? "Empty Prompt" : prompt;
                std::cout << "[SERVER] Received Prompt: " << prompt << std::endl;

                // Form Response
                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: " +
                           std::to_string(prompt.size()) + "\r\n\r\n" +
                           prompt;
            }
        }
    }
    else
    {
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    send(client_socket, response.c_str(), response.size(), 0);
    closesocket(client_socket);
}

int main()
{
    WSADATA wsaData;
    SOCKET server_fd;
    struct sockaddr_in address;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // Initializes the Windows Socket API
        // Required before any socket operations
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Creates a TCP socket (SOCK_STREAM)
    // AF_INET specifies IPv4 addressing

    if (server_fd == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces
    address.sin_port = htons(PORT);       // convert port to network byte order

    // Bind socket
    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true)
    {
        SOCKET client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}