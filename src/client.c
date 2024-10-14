#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct
{
    int socket;
    char username[50];
} User;

char current_user[50] = "";
int is_in_group = 0;
char group_name[50] = "";

void send_command(int sockfd, const char *command)
{
    if (send(sockfd, command, strlen(command), 0) == -1)
    {
        perror("send");
    }

    char buffer[BUFFER_SIZE];
    int nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';
        printf("Server response: %s\n", buffer);
    }
    else if (nbytes == 0)
    {
        printf("Server closed the connection\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        perror("recv");
    }
}

void handle_login_command(int sockfd, char *command)
{
    // Send login command to the server
    if (send(sockfd, command, strlen(command), 0) == -1)
    {
        perror("send");
        return;
    }

    // Wait for server response
    char buffer[BUFFER_SIZE];
    int nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';
        printf("Server response: %s\n", buffer);
        if (strncmp(buffer, "Login successful", 16) == 0)
        {
            sscanf(command, "login %s", current_user);
            printf("logged in as %s\n", current_user);
        }
    }
    else if (nbytes == 0)
    {
        printf("Server closed the connection\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        perror("recv");
    }
}

void handle_chat(int sockfd, char *command)
{
    printf("you are now in : %s", group_name);
    while (1)
    {
        printf("Enter message (type 'exit' to leave group): ");
        if (fgets(command, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "exit", 4) == 0)
        {
            printf("Leaving group...\n");
            is_in_group = 0;
            break;
        }

        if (send(sockfd, command, strlen(command), 0) == -1)
        {
            perror("send");
            break;
        }

        char buffer[BUFFER_SIZE];
        int nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes > 0)
        {
            buffer[nbytes] = '\0';
            printf("Server response: %s\n", buffer);
        }
        else if (nbytes == 0)
        {
            printf("Server closed the connection\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            perror("recv");
            break;
        }
    }
}

void handle_join_command(int sockfd, char *command)
{
    if (strlen(current_user) == 0)
    {
        printf("You must be logged in to join a group.\n\n");
    }
    else
    {
        char join_command[BUFFER_SIZE];
        snprintf(join_command, sizeof(join_command), "join_group %s %s", current_user, command + 11);

        if (send(sockfd, join_command, strlen(join_command), 0) == -1)
        {
            perror("send");
            return;
        }
    }
    char buffer[BUFFER_SIZE];
    int nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("Server response: %s\n", buffer);
        if (strncmp(buffer, "Joined group successfully", 25) == 0)
        {
            sscanf(command, "join_group %s", group_name);
            handle_chat(sockfd, command);
        }
    }
    else if (nbytes == 0)
    {
        printf("Server closed the connection\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        perror("recv");
    }
}

void menu(int sockfd, char *command)
{
    while (1)
    {
        printf("--------------------\nAvailable commands:\n");
        printf("login <username> <password>\n");
        printf("create_user <username> <age> <Gender (M/F)> <password>\n\n");
        printf("commands for logged in users:\n");
        printf("list_groups\n");
        printf("join_group <group_name>\n--------------------\n");
        printf("Enter command: ");
        if (fgets(command, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "exit", 5) == 0)
        {
            return;
        }

        if (strncmp(command, "login", 5) == 0)
        {
            handle_login_command(sockfd, command);
        }
        else if (strncmp(command, "join_group", 10) == 0)
        {
            handle_join_command(sockfd, command);
        }
        else if (strncmp(command, "list_groups", 11) == 0)
        {
            if (strlen(current_user) == 0)
            {
                printf("You must be logged in to list groups.\n\n");
            }
            else
            {
                send_command(sockfd, command);
            }
        }
        else
        {
            send_command(sockfd, command);
        }
    }
}

int main(int argc, char *argv[])
{

    const char *server_ip = "127.0.0.1";
    int server_port = 8080;

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