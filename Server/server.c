#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>

#define BUFFER_SIZE 1024
#define FILE_FOLDER "./root/"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int PORT = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char file_path[PATH_MAX];
    FILE *file;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    read(new_socket, buffer, BUFFER_SIZE);

    // Construct the full file path
    snprintf(file_path, sizeof(file_path), "%s%s", FILE_FOLDER, buffer);

    // Attempt to open the file
    //file = fopen(file_path, "r");
    file = fopen("./root/first.txt","r");
    
    printf("Opening root/%s",buffer);
    
    char buffer1[1024];  // Buffer to store file contents
    while (fgets(buffer1, sizeof(buffer1), file) != NULL) {
        printf("%s\n", buffer1);
    }

    if (file == NULL) {
        perror("Error opening file");
        send(new_socket, "File not found or cannot be opened", 35, 0);
    } else {
        // Read file and send its contents back to the client (optional)
        // For example purposes, let's just confirm the file was opened
        send(new_socket, "File opened successfully", 25, 0);
        fclose(file);
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
