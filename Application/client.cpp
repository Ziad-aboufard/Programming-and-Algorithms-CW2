// client.cpp

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cerr << "Error: Unable to create client socket\n";
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error: Unable to connect to server\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to server on port " << PORT << std::endl;

    // Send username to server
    std::string username = "Ziad";
    send(client_socket, username.c_str(), username.length(), 0);

    // Send password to server
    std::string password = "Ziad";
    send(client_socket, password.c_str(), password.length(), 0);

    // Send message to server
    std::string message = "Hello from client";
    send(client_socket, message.c_str(), message.length(), 0);
    std::cout << "Message sent to server: " << message << std::endl;

    // Receive response from server
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Error: Failed to receive message from server\n";
    }
    else
    {
        buffer[bytes_read] = '\0';
        std::cout << "Received from server: " << buffer << std::endl;
    }

    // Close client socket
    close(client_socket);

    return 0;
}
