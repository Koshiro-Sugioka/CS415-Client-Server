#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int PORT = atoi(argv[1]);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    printf("Connecting the local port: %s\n", argv[1]);

    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Enter the filename to send: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        printf("Error reading input.\n");
        return -1;
    }

    // Remove newline character if present
    //buffer[strcspn(buffer, "\n")] = '\0';  // Null-terminate the string

    // Send the message and check for errors
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        printf("\n Send failed \n");
        return -1;
    }

    memset(buffer, 0, BUFFER_SIZE);  // Clear buffer
    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        printf("\n Read failed \n");
        return -1;
    }
    //printf("Message from server: %s\n", buffer);

    char *newline_pos = strchr(buffer, '\n');

    int length = newline_pos - buffer;
    char line[length + 1];  // +1 for the null terminator

    strncpy(line, buffer, length);
    line[length] = '\0';  // Add null terminator

    printf("File size: %s\n", line);
    int i = 0;
    while(i < atoi(line)){
        printf("%c", buffer[i+length+1]);
        i++;
    }
    printf("\n");
    

    close(sock);
    return 0;
}
