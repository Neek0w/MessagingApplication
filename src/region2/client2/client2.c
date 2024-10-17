
#include "client_utils.h"

// Define server2 IP and port
#define PORT 8081
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE 8192

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