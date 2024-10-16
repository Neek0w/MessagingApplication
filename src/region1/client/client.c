/**
 * @file chat_client.c
 * @brief A simple chat client with file upload and download functionality.
 *
 * This client allows users to log in, join groups, send messages, and upload/download files to/from a server.
 * It uses TCP sockets for communication with the server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>
#include <poll.h>
#include "client_utils.h"

// Define server IP and port
#define PORT 8080
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE 8192

/**
 * @brief The main function for the chat client program.
 *
 * It establishes a connection to the server and provides an interface for the user to log in and interact with groups.
 *
 * @return int 0 on success, non-zero on failure.
 */
int main(int argc, char *argv[])
{

    const char *server_ip = SERVER_IP;
    int server_port = PORT;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char command[BUFFER_SIZE];

    menu(sockfd, command);

    printf("exiting application...\n\n");

    close(sockfd);
    return 0;
}