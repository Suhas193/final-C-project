#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char server_ip[] = "127.0.0.1"; // Server IP address
    int server_port = 8888; // Server port

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        return 1;
    }

    printf("Connected to the server.\n");

    char username[50];
    char password[50];

    printf("Enter your username: ");
    gets(username);

    printf("Enter your password: ");
    gets(password);

    send(client_socket, username, strlen(username), 0);
    send(client_socket, password, strlen(password), 0);

    char buffer[1024];
    int bytes_received;

    // Receive the initial message from the server
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == SOCKET_ERROR) {
        printf("Error in receiving data.\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    } else if (bytes_received == 0) {
        printf("Server disconnected.\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    buffer[bytes_received] = '\0';
    printf("Server: %s\n", buffer);

    while (1) {
        // Prompt the client to send a message
        printf("Client (You): ");
        gets(buffer);

        // Send the client's message to the server
        send(client_socket, buffer, strlen(buffer), 0);

        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == SOCKET_ERROR) {
            printf("Error in receiving data.\n");
            break;
        } else if (bytes_received == 0) {
            printf("Server disconnected.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Server: %s\n", buffer);
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
