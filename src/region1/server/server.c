/**
 * @file server.c
 * @brief This file implements a multi-client server with functionalities for user authentication,
 *        file uploads/downloads, group management, and message handling between clients.
 *
 * The server communicates with a secondary server to ensure data synchronization and load balancing.
 * Clients can perform various actions such as login, user creation, file management, and joining groups.
 * Commands received from clients are handled and processed, with some forwarded to the secondary server.
 *
 * @details
 * - User Authentication: Clients can log in or create new accounts.
 * - Group Management: Clients can create or join groups, list available groups, and exchange messages.
 * - File Management: Clients can upload or download files to/from specific groups.
 * - Server Communication: Handles synchronization between the primary and secondary server.
 * - Handles multiple clients simultaneously using the `poll` mechanism.
 *
 * @note This server listens on two ports (one for clients and one for communication with another server).
 *
 * @dependencies
 * - Standard C libraries: `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<unistd.h>`, `<arpa/inet.h>`,
 *   `<poll.h>`, `<dirent.h>`, `<sys/types.h>`, `<sys/stat.h>`
 * - Custom `database.h`: A custom header for user and group data handling.
 *
 * @authors
 * Author Name: LUNET Louis, EGLOFF Nicolas
 *
 * @date
 * 16/10/2024
 */

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

// define server2 ip and port
#define OTHER_SERVER_PORT 8081
#define OTHER_SERVER_IP "127.0.0.1"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 8192
#define OTHER_SERVER_FD 4

/**
 * @brief Adds a new client to the server.
 *
 * This function adds a new client with a specified username and file descriptor (fd)
 * to the list of active clients, if the maximum number of clients has not been reached.
 *
 * @param username The username of the client.
 * @param fd The file descriptor associated with the client.
 *
 * @note If the maximum number of clients is reached, the client is not added.
 */
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

/**
 * @brief Removes a client from the server.
 *
 * This function removes the client with the specified file descriptor (fd)
 * from the list of active clients.
 *
 * @param fd The file descriptor of the client to be removed.
 */
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

/**
 * @brief Handles the login process for a client.
 *
 * Authenticates the client using the provided username and password. If authentication
 * is successful, the client is added to the active clients list, and a success message
 * is sent. If authentication fails, an error message is sent to the client.
 *
 * @param client_fd The file descriptor of the client attempting to log in.
 * @param username The username provided by the client.
 * @param password The password provided by the client.
 */
void handle_login(int client_fd, char *username, char *password)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {

            send(client_fd, "Login successful\n", 17, 0);
            add_client(username, client_fd);
            return;
        }
    }

    send(client_fd, "Login failed\n", 13, 0);
}

/**
 * @brief Creates a new user.
 *
 * This function allows a client to create a new user account with the specified
 * username, gender, age, and password. If the username already exists, an error
 * message is sent to the client.
 *
 * @param client_fd The file descriptor of the client.
 * @param username The desired username for the new account.
 * @param gender The gender of the new user.
 * @param age The age of the new user.
 * @param password The password for the new account.
 */
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

/**
 * @brief Handles the file upload process for a client.
 *
 * Receives a file from the client and stores it in the server's group-specific directory.
 * It verifies the group existence, receives the file size, and writes the data to disk.
 *
 * @param client_fd The file descriptor of the client.
 * @param group_name The name of the group the file belongs to.
 * @param file_name The name of the file being uploaded.
 *
 * @note If the group does not exist or any error occurs, the client is notified.
 */
void handle_upload_file(int client_fd, const char *group_name, const char *file_name)
{
    printf("uploading file... \n");

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

    // Notify the client that the server is ready
    send(client_fd, "SERVER_READY\n", 13, 0);

    // Receive the file size from the client
    uint64_t file_size;
    if (recv(client_fd, &file_size, sizeof(file_size), 0) <= 0)
    {
        perror("recv (file size)");
        fclose(file);
        return;
    }
    printf("File size: %lu bytes\n", file_size);

    // Notify the client that the file size is received and OK
    send(client_fd, "SIZE_OK\n", 8, 0);

    // Receive the file data from the client
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    uint64_t total_bytes_received = 0;
    while (total_bytes_received < file_size && (bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
        total_bytes_received += bytes_received;
    }

    if (total_bytes_received == file_size)
    {
        printf("File received successfully: %s\n", file_path);
    }
    else
    {
        printf("File receive incomplete. Received %lu of %lu bytes.\n", total_bytes_received, file_size);
    }

    fclose(file);
}

/**
 * @brief Handles the file download process for a client.
 *
 * Sends a requested file to the client. The file must be located in the group-specific
 * directory on the server.
 *
 * @param client_fd The file descriptor of the client.
 * @param group_name The name of the group the file belongs to.
 * @param file_name The name of the file to be downloaded.
 *
 * @note If the file cannot be opened, the client is notified.
 */
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

    // Send the file size to the client
    if (send(client_fd, &file_size, sizeof(file_size), 0) == -1)
    {
        perror("send (file size)");
        fclose(file);
        return;
    }

    // Send the file data to the client
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(client_fd, buffer, bytes_read, 0) == -1)
        {
            perror("send (file data)");
            break;
        }
    }

    fclose(file);
    printf("File sent successfully\n");
}

/**
 * @brief Lists all files in a group's directory for a client.
 *
 * Sends a list of files present in the group's directory to the requesting client.
 *
 * @param client_fd The file descriptor of the client.
 * @param group_name The name of the group whose files are to be listed.
 *
 * @note If the group folder cannot be opened, the client is notified.
 */
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

/**
 * @brief Lists all available groups on the server.
 *
 * Sends a list of all groups on the server to the requesting client.
 *
 * @param client_fd The file descriptor of the client.
 */
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

/**
 * @brief Adds a user to a group.
 *
 * This function allows a client to join a group. If the user is already a member or the
 * group is full, an appropriate message is sent to the client.
 *
 * @param client_fd The file descriptor of the client.
 * @param username The username of the client attempting to join the group.
 * @param group_name The name of the group to join.
 *
 * @note If the group does not exist, the client is notified.
 */
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

/**
 * @brief Retrieves the file descriptor of a client by their username.
 *
 * Searches through the list of active clients and returns the file descriptor (fd)
 * associated with the given username.
 *
 * @param username The username of the client.
 * @return int The file descriptor of the client, or -1 if the client is not found.
 */
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

/**
 * @brief Handles messages sent within a group.
 *
 * Depending on the message type, this function either removes a user from a group or sends
 * a message to all group members, excluding the sender.
 *
 * @param client_fd The file descriptor of the client.
 * @param group The name of the group.
 * @param user The username of the client.
 * @param message The message to be sent (for type 1).
 * @param type The type of the message (0 for leave group, 1 for send message).
 *
 * @note If the client is not part of the group, an error message is sent.
 */
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
                    if (client_fd == OTHER_SERVER_FD)
                        send(client_fd, "Left group successfully\n", 24, 0);
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
                    if (client_fd == OTHER_SERVER_FD)
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

/**
 * @brief Handles commands from a client.
 *
 * This function processes incoming client commands, such as login, user creation,
 * file upload/download, group joining, and messaging. Some commands may be forwarded
 * to a secondary server for processing.
 *
 * @param client_fd The file descriptor of the client.
 * @param second_server_fd The file descriptor for the connection to the secondary server.
 *
 * @note If a client disconnects, it is removed from the active client list.
 */

void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];
    printf("\n\nAwaiting for recv ...\n\n");
    int nbytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0)
    {
        buffer[nbytes] = '\0';

        if (client_fd == OTHER_SERVER_FD)
        {
            printf("Received message from server: %s\n", buffer);
        }
        else
        {
            printf("Received message from client %d: %s\n", client_fd, buffer);
        }

        if (client_fd != OTHER_SERVER_FD &&
            (strncmp(buffer, "join_group", 10) == 0 ||
             strncmp(buffer, "message", 7) == 0 ||
             strncmp(buffer, "create_user", 11) == 0))
        {
            // Forward the command to the second server
            printf("sending command to server\n");
            if (send(OTHER_SERVER_FD, buffer, nbytes, 0) == -1)
            {
                perror("send to second server");
            }

            // Receive response from the second server
            char response[BUFFER_SIZE];
            printf("\n\nAwaiting for recv ...\n\n");

            int response_bytes = recv(OTHER_SERVER_FD, response, sizeof(response) - 1, 0);
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

        if (sscanf(buffer, "%49s %49s %49s %d %[^\n]", command, arg1, arg2, &arg4, arg3) >= 1)
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

                handle_upload_file(client_fd, arg1, arg2);
                if (client_fd != OTHER_SERVER_FD)
                {
                    printf("tranferring file to other server\n");
                    char transfer_command[BUFFER_SIZE];
                    snprintf(transfer_command, sizeof(transfer_command), "transfer_file %s %s\n", arg1, arg2);
                    send(OTHER_SERVER_FD, transfer_command, strlen(transfer_command), 0);
                    handle_download_file(OTHER_SERVER_FD, arg1, arg2);
                }
            }
            else if (strcmp(command, "list_files") == 0)
            {
                handle_list_files(client_fd, arg1); // arg1 is group name
            }
            else if (strcmp(command, "download_file") == 0)
            {
                handle_download_file(client_fd, arg1, arg2);
            }
            else if (client_fd == OTHER_SERVER_FD && strcmp(command, "transfer_file") == 0)
            {
                handle_upload_file(OTHER_SERVER_FD, arg1, arg2);
            }
            else
            {
                if (client_fd != OTHER_SERVER_FD)

                    send(client_fd, "Unknown command\n", 16, 0);
            }
        }
        else
        {
            if (client_fd != OTHER_SERVER_FD)

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
    memset(buffer, 0, sizeof(buffer));
}

/**
 * @brief Prints the current server state.
 *
 * This function displays the list of users, groups, and active clients on the server.
 *
 * @note This is primarily for debugging and monitoring purposes.
 */
void print_data()
{
    printf("\n\nServer state\n----------------------------------------------\n");
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
    printf("---------------------------------------------------\n");
}

/**
 * @brief Main entry point for the server application.
 *
 * Initializes the server, sets up socket connections, and handles incoming client requests
 * using a polling mechanism. The server also communicates with a secondary server to manage
 * load balancing and synchronization.
 *
 * @return int Returns 0 on successful execution, or exits with an error code.
 */
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
    int other_server_fd;
    struct sockaddr_in second_server_address;

    if ((other_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    second_server_address.sin_family = AF_INET;
    second_server_address.sin_addr.s_addr = inet_addr(OTHER_SERVER_IP); // Replace with the actual IP address
    second_server_address.sin_port = htons(OTHER_SERVER_PORT);          // Replace with the actual port

    if (connect(other_server_fd, (struct sockaddr *)&second_server_address, sizeof(second_server_address)) < 0)
    {
        perror("connect failed");
        close(other_server_fd);
        exit(EXIT_FAILURE);
    }

    fds[1].fd = other_server_fd;
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
                handle_client(fds[i].fd);
                print_data();
            }
        }
    }

    close(server_fd);

    return 0;
}