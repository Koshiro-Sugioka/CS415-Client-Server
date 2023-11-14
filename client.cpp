#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Convert IPv4 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return -1;
    }

    while (true) {
        std::cout << "Client: ";
        std::string client_message;
        std::getline(std::cin, client_message);
        //sending a message to server
        send(sock, client_message.c_str(), client_message.size(), 0);
        memset(buffer, 0, sizeof(buffer));
        //reading the message sent by server
        int read_val = read(sock, buffer, 1024);
        if (read_val <= 0) {
            break;
        }

        std::cout << "Server: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
