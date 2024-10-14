#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "database.h"

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

void handle_login(int client_fd, char *username, char *password)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            send(client_fd, "Login successful\n", 17, 0);
            return;
        }
    }
    send(client_fd, "Login failed\n", 13, 0);
}

void handle_create_user(int client_fd, char *username, char *gender, int age, char *password)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            send(client_fd, "Username already exists\n", 24, 0);
            return;
        }
    }
    add_user(username, gender[0], age, password);
    send(client_fd, "User created successfully\n", 26, 0);
}

void handle_list_groups(int client_fd)
{
    char buffer[BUFFER_SIZE] = "Groups:\n";
    for (int i = 0; i < group_count; i++)
    {
        strcat(buffer, groups[i].group_name);
        strcat(buffer, "\n");
    }
    send(client_fd, buffer, strlen(buffer), 0);
}

void handle_join_group(int client_fd, char *username, char *group_name)
{
    for (int i = 0; i < group_count; i++)
    {
        if (strcmp(groups[i].group_name, group_name) == 0)
        {
            for (int j = 0; j < groups[i].member_count; j++)
            {
                if (strcmp(groups[i].members[j], username) == 0)
                {
                    send(client_fd, "Already in the group\n", 21, 0);
                    return;
                }
            }
            if (groups[i].member_count < MAX_GROUP_MEMBERS)
            {
                strcpy(groups[i].members[groups[i].member_count], username);
                groups[i].member_count++;
                send(client_fd, "Joined group successfully\n", 26, 0);
            }
            else
            {
                send(client_fd, "Group is full\n", 14, 0);
            }
            return;
        }
    }
    send(client_fd, "Group not found\n", 16, 0);
}

void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];
    int nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';
        printf("Received message from client %d: %s\n", client_fd, buffer);

        char command[50], arg1[50], arg2[50], arg3[50];
        int arg4;

        if (sscanf(buffer, "%s %s %s %d %s", command, arg1, arg2, &arg4, arg3) >= 1)
        {
            if (strcmp(command, "login") == 0)
            {
                handle_login(client_fd, arg1, arg2);
            }
            else if (strcmp(command, "create_user") == 0)
            {
                handle_create_user(client_fd, arg1, arg2, arg4, arg3);
            }
            else if (strcmp(command, "list_groups") == 0)
            {
                handle_list_groups(client_fd);
            }
            else if (strcmp(command, "join_group") == 0)
            {
                handle_join_group(client_fd, arg1, arg2);
            }
            else
            {
                send(client_fd, "Unknown command\n", 16, 0);
            }
        }
        else
        {
            send(client_fd, "Invalid command format\n", 23, 0);
        }
    }
    else if (nbytes == 0)
    {
        printf("Client %d disconnected\n", client_fd);
        close(client_fd);
    }
    else
    {
        perror("recv");
    }
}

void print_data()
{
    printf("Users:\n");
    for (int i = 0; i < user_count; i++)
    {
        printf("Username: %s, Gender: %c, Age: %d, Password: %s\n",
               users[i].username, (int)users[i].gender, users[i].age, users[i].password);
    }

    printf("\nGroups:\n");
    for (int i = 0; i < group_count; i++)
    {
        printf("Group: %s, Members: ", groups[i].group_name);
        for (int j = 0; j < groups[i].member_count; j++)
        {
            printf("%s ", groups[i].members[j]);
        }
        printf("\n");
    }
}

void handle_message(int client_fd, char *username, char *group_name, char *message)
{
    for (int i = 0; i < group_count; i++)
    {
        if (strcmp(groups[i].group_name, group_name) == 0)
        {
            for (int j = 0; j < groups[i].member_count; j++)
            {
                if (strcmp(groups[i].members[j], username) == 0)
                {
                    char buffer[BUFFER_SIZE];
                    snprintf(buffer, sizeof(buffer), "%s: %s\n", username, message);
                    for (int k = 0; k < groups[i].member_count; k++)
                    {
                        if (strcmp(groups[i].members[k], username) != 0)
                        {
                            int member_fd = get_client_fd_by_username(groups[i].members[k]);
                            if (member_fd != -1)
                            {
                                send(member_fd, buffer, strlen(buffer), 0);
                            }
                        }
                    }
                    return;
                }
            }
            send(client_fd, "You are not a member of this group\n", 35, 0);
            return;
        }
    }
    send(client_fd, "Group not found\n", 16, 0);
}

int main()
{
    parse_file("data.txt");
    print_data();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 1; i < MAX_CLIENTS; i++)
    {
        fds[i].fd = -1;
    }

    while (1)
    {
        int activity = poll(fds, nfds, -1);

        if (activity < 0)
        {
            perror("poll error");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                continue;
            }

            printf("New connection, socket fd is %d, ip is : %s, port : %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 1; i < MAX_CLIENTS; i++)
            {
                if (fds[i].fd == -1)
                {
                    fds[i].fd = new_socket;
                    fds[i].events = POLLIN;
                    if (i >= nfds)
                    {
                        nfds = i + 1;
                    }
                    break;
                }
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].fd != -1 && fds[i].revents & POLLIN)
            {
                handle_client(fds[i].fd);
                print_data();
            }
        }
    }

    close(server_fd);

    return 0;
}