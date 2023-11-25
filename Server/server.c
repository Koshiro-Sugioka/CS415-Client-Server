#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>


#define BUFFER_SIZE 1024

#define FILE_FOLDER "./root/"

void* handle_request(void *socket_fd_ptr) {
    printf("New Connection\n");
    int socket_fd = *((int*)socket_fd_ptr);
    free(socket_fd_ptr); 
    
    char filename_buffer[BUFFER_SIZE] = {0};

    // Receive the filename from the client
    read(socket_fd, filename_buffer, BUFFER_SIZE);
    filename_buffer[strcspn(filename_buffer, "\n")] = '\0'; // Remove potential newline

    // Construct the full file path
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s%s", FILE_FOLDER, filename_buffer);

    // Open the file
    printf("Opening root/%s\n", filename_buffer);
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        send(socket_fd, "File not found or cannot be opened\n", 35, 0);
        return;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    printf("Size is %ld bytes.\n", file_size);
    fseek(file, 0, SEEK_SET);

    // Send the file size to the client
    char size_msg[BUFFER_SIZE];
    snprintf(size_msg, sizeof(size_msg), "%ld\n", file_size); // Include newline as a flag
    send(socket_fd, size_msg, strlen(size_msg), 0);

    // Read the file and send its contents
    char file_content[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(file_content, 1, BUFFER_SIZE, file)) > 0) {
        send(socket_fd, file_content, bytes_read, 0);
    }

    // Clean up
    fclose(file);
    
    close(socket_fd); // Close the client socket
    return NULL;
}


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
    
    while (1) {
        int *new_socket_ptr = malloc(sizeof(int));
        if (new_socket_ptr == NULL) {
            perror("Failed to allocate memory for socket descriptor");
            continue;
        }

        *new_socket_ptr = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (*new_socket_ptr < 0) {
            perror("accept");
            free(new_socket_ptr);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_request, new_socket_ptr) != 0) {
            perror("Failed to create thread");
            close(*new_socket_ptr);
            free(new_socket_ptr);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
