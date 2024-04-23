// server.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERS_FILE "users.txt"
#define CHAT_LOG_FILE "chat_log.txt"

bool authenticate_user(const std::string &username, const std::string &password)
{
    std::ifstream users_file(USERS_FILE);
    std::string line;

    while (std::getline(users_file, line))
    {
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos)
        {
            std::string stored_username = line.substr(0, delimiter_pos);
            std::string stored_password = line.substr(delimiter_pos + 1);
            if (stored_username == username && stored_password == password)
            {
                return true;
            }
        }
    }

    return false;
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Receive username from client
    bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Error: Failed to receive username from client\n";
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';
    std::string username(buffer);

    // Receive password from client
    bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Error: Failed to receive password from client\n";
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';
    std::string password(buffer);

    // Authenticate user
    if (!authenticate_user(username, password))
    {
        std::cerr << "Error: Authentication failed for user: " << username << std::endl;
        close(client_socket);
        return;
    }

    std::cout << "User authenticated: " << username << std::endl;

    // Receive message from client
    bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Error: Failed to receive message from client\n";
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';
    std::string message(buffer);

    // Write message to chat log file
    std::ofstream chat_log(CHAT_LOG_FILE, std::ios_base::app);
    if (!chat_log.is_open())
    {
        std::cerr << "Error: Failed to open chat log file\n";
        close(client_socket);
        return;
    }
    chat_log << username << ": " << message << std::endl;
    chat_log.close();

    // Send response back to client
    std::string response = "Message received: ";
    response += message;
    send(client_socket, response.c_str(), response.length(), 0);

    // Close client socket
    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Error: Unable to create server socket\n";
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind server socket to the address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error: Unable to bind server socket\n";
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_socket, 5) < 0)
    {
        std::cerr << "Error: Unable to listen for connections\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Accept incoming connections and handle them
    std::vector<std::thread> client_threads;
    while (true)
    {
        // Accept connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            std::cerr << "Error: Unable to accept connection\n";
            continue;
        }
        std::cout << "Connection accepted from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;

        // Start a new thread to handle client communication
        client_threads.emplace_back(std::thread(handle_client, client_socket));
    }

    // Join all client threads
    for (auto &thread : client_threads)
    {
        thread.join();
    }

    // Close server socket
    close(server_socket);

    return 0;
}
