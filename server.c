#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define USERNAME "admin"
#define PASSWORD "pass"

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Binding failed.\n");
        return 1;
    }

    listen(server_socket, 5);
    printf("Server listening on port 8888...\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Failed to accept connection.\n");
            continue; // Continue listening for new connections
        }

        printf("Client connected.\n");

        // Authentication
        char username[50];
        char password[50];

        if (recv(client_socket, username, sizeof(username), 0) == SOCKET_ERROR) {
            printf("Error in receiving username.\n");
            closesocket(client_socket);
            continue; // Continue listening for new connections
        }

        if (recv(client_socket, password, sizeof(password), 0) == SOCKET_ERROR) {
            printf("Error in receiving password.\n");
            closesocket(client_socket);
            continue; // Continue listening for new connections
        }

        username[strlen(username) - 1] = '\0'; // Remove newline character
        password[strlen(password) - 1] = '\0'; // Remove newline character

        // Debugging output to verify received credentials
        printf("Received Username: %s\n", username);
        printf("Received Password: %s\n", password);

        if (strncmp(username, USERNAME, strlen(USERNAME)) != 0 || strncmp(password, PASSWORD, strlen(PASSWORD)) != 0) {
            printf("Authentication failed. Closing connection.\n");
            closesocket(client_socket);
            continue; // Continue listening for new connections
        }

        printf("Authentication successful.\n");

        // Chat with the client
        char buffer[1024];
        int bytes_received;

        // Prompt the server to send an initial message
        printf("Server (You): ");
        gets(buffer);

        // Send the server's initial message to the client
        send(client_socket, buffer, strlen(buffer), 0);

        while (1) {
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received == SOCKET_ERROR) {
                printf("Error in receiving data.\n");
                break;
            } else if (bytes_received == 0) {
                printf("Client disconnected.\n");
                break;
            }

            buffer[bytes_received] = '\0';
            printf("Client: %s\n", buffer);

            // Prompt the server to send another message
            printf("Server (You): ");
            gets(buffer);

            // Send the server's message to the client
            send(client_socket, buffer, strlen(buffer), 0);
        }

        // Close the client socket
        closesocket(client_socket);
    }

    // Cleanup and server exit code go here...
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
