#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <CommCtrl.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

const char *SERVER_IP = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

// GUI Controls
HWND hEditMessages, hEditInput, hBtnSend;
SOCKET client_socket;
std::thread receiver_thread;
bool running = true;

void AddMessage(const std::string &message)
{
    SendMessageA(hEditMessages, EM_SETSEL, -1, -1);
    SendMessageA(hEditMessages, EM_REPLACESEL, FALSE, (LPARAM)(message + "\r\n").c_str());
}

void SendMessageToServer()
{
    char buffer[BUFFER_SIZE];
    GetWindowTextA(hEditInput, buffer, BUFFER_SIZE);
    std::string message(buffer);

    if (!message.empty())
    {
        message += "\n\n";
        send(client_socket, message.c_str(), message.size(), 0);
        SetWindowTextA(hEditInput, "");
    }
}

LRESULT CALLBACK InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (msg == WM_KEYDOWN && wParam == VK_RETURN)
    {
        SendMessageToServer();
        return 0;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

void ReceiverThread()
{
    char buffer[BUFFER_SIZE];
    while (running)
    {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            std::string msg(buffer);
            size_t pos = msg.find("\n\n");
            if (pos != std::string::npos)
            {
                msg = msg.substr(0, pos);
                AddMessage(msg);
            }
        }
        else
        {
            running = false;
            PostMessage(GetParent(hEditMessages), WM_CLOSE, 0, 0);
            break;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create GUI controls
        hEditMessages = CreateWindowA("EDIT", "",
                                      WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                      10, 10, 460, 200, hWnd, NULL, NULL, NULL);

        hEditInput = CreateWindowA("EDIT", "",
                                   WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
                                   10, 220, 380, 25, hWnd, NULL, NULL, NULL);

        hBtnSend = CreateWindowA("BUTTON", "Send",
                                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 400, 220, 70, 25, hWnd, (HMENU)1, NULL, NULL);

        // Set input focus and subclass
        SetFocus(hEditInput);
        SetWindowSubclass(hEditInput, InputWndProc, 0, 0);

        // Connect to server
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr));

        receiver_thread = std::thread(ReceiverThread);
        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1)
        {
            SendMessageToServer();
        }
        break;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        MoveWindow(hEditMessages, 10, 10, width - 20, height - 80, TRUE);
        MoveWindow(hEditInput, 10, height - 60, width - 90, 25, TRUE);
        MoveWindow(hBtnSend, width - 80, height - 60, 70, 25, TRUE);
        break;
    }

    case WM_CLOSE:
        running = false;
        closesocket(client_socket);
        if (receiver_thread.joinable())
            receiver_thread.join();
        WSACleanup();
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChatClientClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    HWND hWnd = CreateWindowA("ChatClientClass", "Chat Client",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
                              NULL, NULL, hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}