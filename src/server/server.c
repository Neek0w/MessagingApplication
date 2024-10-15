#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "database.h"

#define PORT 8080
#define SECOND_SERVER_PORT 8081
#define SECOND_SERVER_IP "127.0.0.1"
#define MAX_CLIENTS 100
#define BUFFER_SIZE 8192
#define SECOND_SERVER_FD 4

void add_client(const char *username, int fd)
{
    if (client_count < MAX_CLIENTS)
    {
        strncpy(clients[client_count].username, username, sizeof(clients[client_count].username) - 1);
        clients[client_count].fd = fd;
        client_count++;
    }
    else
    {
        printf("Max clients reached, cannot add more clients.\n");
    }
}

void remove_client(int fd)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].fd == fd)
        {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
}

// AUTHENTIFICATION

void handle_login(int client_fd, char *username, char *password)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            send(client_fd, "Login successful\n", 17, 0);
            if (client_fd != SECOND_SERVER_FD)
                add_client(username, client_fd);
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

void handle_upload_file(int client_fd, const char *group_name, const char *file_name)
{
    // Find the group
    int group_index = -1;
    for (int i = 0; i < group_count; i++)
    {
        if (strcmp(groups[i].group_name, group_name) == 0)
        {
            group_index = i;
            break;
        }
    }

    if (group_index == -1)
    {
        send(client_fd, "Group not found\n", 16, 0);
        return;
    }

    // Create the file path
    char file_path[BUFFER_SIZE];
    snprintf(file_path, sizeof(file_path), "./drive/%s/%s", group_name, file_name);

    // Open the file for writing
    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        perror("fopen");
        send(client_fd, "Error opening file\n", 19, 0);
        return;
    }

    // Receive the file size from the client
    size_t file_size;
    if (recv(client_fd, &file_size, sizeof(file_size), 0) <= 0)
    {
        perror("recv");
        fclose(file);
        return;
    }
    printf("File size: %zu bytes\n", file_size);

    // Receive the file data from the client
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    size_t total_bytes_received = 0;
    while (total_bytes_received < file_size && (bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
        total_bytes_received += bytes_received;
    }

    if (bytes_received < 0)
    {
        perror("recv");
    }

    fclose(file);
    printf("File received and saved to %s\n", file_path);
    send(client_fd, "File uploaded successfully\n", 27, 0);
}

void handle_download_file(int client_fd, const char *group_name, const char *file_name)
{
    // Create the file path
    char file_path[BUFFER_SIZE];
    snprintf(file_path, sizeof(file_path), "./drive/%s/%s", group_name, file_name);

    // Open the file for reading
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("fopen");
        send(client_fd, "Error opening file\n", 19, 0);
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    uint64_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("File size: %zu bytes\n", file_size);

    // Send the file size to the client
    if (send(client_fd, &file_size, sizeof(file_size), 0) == -1)
    {
        perror("send");
        fclose(file);
        return;
    }

    // Read the file and send its contents to the client
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        printf("\n\nBuffer :\n %s", buffer);
        if (send(client_fd, buffer, bytes_read, 0) == -1)
        {
            perror("send");
            break;
        }
    }

    if (ferror(file))
    {
        perror("fread");
    }

    fclose(file);
    printf("File sent successfully\n");
}

void handle_list_files(int client_fd, const char *group_name)
{
    char folder_path[BUFFER_SIZE];
    snprintf(folder_path, sizeof(folder_path), "./drive/%s", group_name);

    DIR *dir = opendir(folder_path);
    if (dir == NULL)
    {
        perror("opendir");
        send(client_fd, "Error opening group folder\n", 27, 0);
        return;
    }

    struct dirent *entry;
    char buffer[BUFFER_SIZE] = "Files:\n";
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] != '.')
        {
            strcat(buffer, entry->d_name);
            strcat(buffer, "\n");
        }
    }
    closedir(dir);

    // Send the list of files to the client
    if (send(client_fd, buffer, strlen(buffer), 0) == -1)
    {
        perror("send");
    }
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

int get_client_fd_by_username(const char *username)
{
    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            return clients[i].fd;
        }
    }
    return -1;
}

void handle_message(int client_fd, char *group, char *user, char *message, int type)
{

    if (type == 0)
    {
        for (int i = 0; i < group_count; i++)
        {
            for (int j = 0; j < groups[i].member_count; j++)
            {
                if (strcmp(groups[i].members[j], user) == 0)
                {
                    for (int k = j; k < groups[i].member_count - 1; k++)
                    {
                        strcpy(groups[i].members[k], groups[i].members[k + 1]);
                    }
                    groups[i].member_count--;
                    // remove_client(client_fd);
                    return;
                }
            }
        }
    }
    else if (type == 1)
    {
        for (int i = 0; i < group_count; i++)
        {
            for (int j = 0; j < groups[i].member_count; j++)
            {
                if (strcmp(groups[i].members[j], user) == 0)
                {
                    for (int k = 0; k < groups[i].member_count; k++)
                    {
                        if (strcmp(groups[i].members[k], user) != 0)
                        {
                            int member_fd = get_client_fd_by_username(groups[i].members[k]);
                            if (member_fd != -1)
                            {
                                printf("Sending message to %s\n fd : %d", groups[i].members[k], member_fd);
                                char send_message[BUFFER_SIZE];
                                snprintf(send_message, sizeof(send_message), "%s: %s", user, message);
                                send(member_fd, send_message, strlen(send_message), 0);
                            }
                        }
                    }
                    if (client_fd == SECOND_SERVER_FD)
                        send(client_fd, "Message sent successfully\n", 26, 0);
                    return;
                }
            }
        }
        send(client_fd, "Invalid command format\n", 23, 0);
    }
    else
    {
        send(client_fd, "Invalid command format\n", 23, 0);
    }
}

void handle_client(int client_fd, int second_server_fd)
{
    char buffer[BUFFER_SIZE];
    int nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';
        printf("Received message from client %d: %s\n", client_fd, buffer);

        if (client_fd != SECOND_SERVER_FD)
        {
            // Forward the command to the second server
            if (send(second_server_fd, buffer, nbytes, 0) == -1)
            {
                perror("send to second server");
            }

            // Receive response from the second server
            char response[BUFFER_SIZE];
            int response_bytes = recv(second_server_fd, response, sizeof(response) - 1, 0);
            if (response_bytes > 0)
            {
                response[response_bytes] = '\0';
                printf("Received response from second server: %s\n", response);
            }
            else if (response_bytes == 0)
            {
                printf("Second server disconnected\n");
            }
            else
            {
                perror("recv from second server");
            }
        }

        char command[50], arg1[50], arg2[50];
        char arg3[BUFFER_SIZE];
        int arg4;

        if (sscanf(buffer, "%s %s %s %d %[^\n]", command, arg1, arg2, &arg4, arg3) >= 1)
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
            else if (strcmp(command, "message") == 0)
            {
                handle_message(client_fd, arg1, arg2, arg3, arg4);
            }
            else if (strcmp(command, "upload_file") == 0)
            {
                handle_upload_file(client_fd, arg1, arg2); // arg1 is group name, arg2 is file name
            }
            else if (strcmp(command, "list_files") == 0)
            {
                handle_list_files(client_fd, arg1); // arg1 is group name
            }
            else if (strcmp(command, "download_file") == 0)
            {
                handle_download_file(client_fd, arg1, arg2); // arg1 is file name
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
        remove_client(client_fd);
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

    printf("\nClients:\n");
    for (int i = 0; i < client_count; i++)
    {
        printf("Username: %s, FD: %d\n", clients[i].username, clients[i].fd);
    }
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
    int nfds = 2;

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    // Initialize the second fd to connect to another server
    int second_server_fd;
    struct sockaddr_in second_server_address;

    if ((second_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    second_server_address.sin_family = AF_INET;
    second_server_address.sin_addr.s_addr = inet_addr(SECOND_SERVER_IP); // Replace with the actual IP address
    second_server_address.sin_port = htons(SECOND_SERVER_PORT);          // Replace with the actual port

    if (connect(second_server_fd, (struct sockaddr *)&second_server_address, sizeof(second_server_address)) < 0)
    {
        perror("connect failed");
        close(second_server_fd);
        exit(EXIT_FAILURE);
    }

    fds[1].fd = second_server_fd;
    fds[1].events = POLLIN;

    for (int i = 2; i < MAX_CLIENTS; i++)
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
                handle_client(fds[i].fd, second_server_fd);
                print_data();
            }
        }
    }

    close(server_fd);

    return 0;
}