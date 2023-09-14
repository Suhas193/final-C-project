#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

struct User {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
};

struct User users[MAX_USERS];
int num_users = 0;

void loadUserCredentials(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open user credentials file.\n");
        return;
    }

    char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 2]; // +2 for space and newline
    while (num_users < MAX_USERS && fgets(line, sizeof(line), file) != NULL) {
        printf("Read Line: %s", line); // Print the line for debugging
        sscanf(line, "%s %s", users[num_users].username, users[num_users].password);
        num_users++;
    }

    fclose(file);
}

int authenticateUser(const char* username, const char* password) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(username, users[i].username) == 0 && strcmp(password, users[i].password) == 0) {
            return 1; // Authentication successful
        }
    }
    return 0; // Authentication failed
}


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

    loadUserCredentials("cred.txt"); // Load user credentials from file

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
        char username[MAX_USERNAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];

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

        for (int i = 0; i < num_users; i++) {
        printf("Stored Username: %s\n", users[i].username);
        printf("Stored Password: %s\n", users[i].password);
        }

        if (!authenticateUser(username, password)) {
            printf("Authentication failed. Closing connection.\n");
            closesocket(client_socket);
            continue; // Continue listening for new connections
        }

        printf("Authentication successful.\n");

        // Chat with the client
        char buffer[1024];
        int bytes_received;

        // Prompt in command
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

        closesocket(client_socket);
    }

    // Cleanup and server exit code.
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
