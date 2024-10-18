#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>
#include <poll.h>
#include <stdio.h>
#include "client_utils.h"
#include "socket_utils.h"

#define BUFFER_SIZE 8192

char current_user[50] = ""; /**< Currently logged-in user's name */
int is_in_group = 0;        /**< Flag indicating if the user is in a group */
char group_name[50] = "";   /**< Name of the group the user is currently in */

/**
 * @brief Send a command to the server and receive a response.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param command The command string to send.
 */
void send_command(int sockfd, char *command)
{
    send_message(sockfd, command, strlen(command), 0);

    char buffer[BUFFER_SIZE];

    receive_message(sockfd, buffer, sizeof(buffer) - 1, 0);
    printf("Server response: %s\n", buffer);
}

/**
 * @brief Handle the login command by sending login credentials to the server.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param command The login command string.
 */
void handle_login_command(int sockfd, char *command)
{
    send_message(sockfd, command, strlen(command), 0);

    // Wait for server response
    char buffer[BUFFER_SIZE];
    receive_message(sockfd, buffer, sizeof(buffer) - 1, 0);
    printf("Server response: %s\n", buffer);
    if (strncmp(buffer, "Login successful", 16) == 0)
    {
        sscanf(command, "login %s", current_user);
        printf("logged in as %s\n", current_user);
    }
}

/**
 * @brief Upload a file to the server under a specific group.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param group_name The name of the group to upload the file to.
 * @param file_path The path to the file to upload.
 */
void upload_file(int sockfd, char *group_name, char *file_path)
{

    printf("uploading file to server ...\n");
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    char *file_name = basename((char *)file_path);

    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "upload_file %s %s", group_name, file_name);

    send_message(sockfd, command, strlen(command), 0);
    printf("command sent\n");

    char server_ready[BUFFER_SIZE];
    int nbytes = recv(sockfd, server_ready, sizeof(server_ready) - 1, 0);
    if (nbytes > 0)
    {
        server_ready[nbytes] = '\0';
        if (strncmp(server_ready, "SERVER_READY", 12) != 0)
        {
            printf("Server is not ready for file upload. Aborting upload.\n");
            fclose(file);
            return;
        }
    }
    else if (nbytes == 0)
    {
        printf("Server closed the connection\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    else
    {
        perror("recv");
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (send(sockfd, &file_size, sizeof(file_size), 0) == -1)
    {
        perror("send");
        fclose(file);
        return;
    }

    char size_ok[BUFFER_SIZE];
    nbytes = recv(sockfd, size_ok, sizeof(size_ok) - 1, 0);
    if (nbytes > 0)
    {
        size_ok[nbytes] = '\0';
        if (strncmp(size_ok, "SIZE_OK", 7) != 0)
        {
            printf("Server did not acknowledge file size. Aborting upload.\n");
            fclose(file);
            return;
        }
    }
    else if (nbytes == 0)
    {
        printf("Server closed the connection\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    else
    {
        perror("recv");
        fclose(file);
        return;
    }

    printf("Uploading file %s to group %s...\n", file_name, group_name);

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(sockfd, buffer, bytes_read, 0) == -1)
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
    printf("File uploaded successfully\n");
}

/**
 * @brief Download a file from the server.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param group_name The name of the group from which to download the file.
 * @param file_name The name of the file to download.
 */
void download_file(int sockfd, char *group_name, char *file_name)
{
    printf("Downloading file %s from group %s...\n", file_name, group_name);

    char download_command[BUFFER_SIZE];
    snprintf(download_command, sizeof(download_command), "download_file %s %s", group_name, file_name);
    send_message(sockfd, download_command, strlen(download_command), 0);

    // Construct the full file path
    char file_path[BUFFER_SIZE];
    snprintf(file_path, sizeof(file_path), "./downloads/%s", file_name);

    // Ouvrir le fichier pour écriture
    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    // Notify the client that the server is ready
    send(sockfd, "SERVER_READY", 12, 0);
    printf("Server ready to receive file\n");

    // Receive the file size from the client
    uint64_t file_size;
    if (recv(sockfd, &file_size, sizeof(file_size), 0) <= 0)
    {
        perror("recv (file size)");
        fclose(file);
        return;
    }
    printf("File size received: %lu\n", file_size);

    // Notify the client that the file size is received and OK
    send(sockfd, "SIZE_OK", 7, 0);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    uint64_t total_bytes_received = 0;
    while (total_bytes_received < file_size)
    {
        bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            perror("recv (file data)");
            break;
        }

        size_t bytes_written = fwrite(buffer, 1, bytes_received, file);
        if (bytes_written != bytes_received)
        {
            perror("fwrite");
            break;
        }

        total_bytes_received += bytes_received;
        printf("Progress: %ld/%ld bytes downloaded\n", total_bytes_received, file_size);
    }

    if (total_bytes_received == file_size)
    {
        printf("File downloaded successfully: %s\n", file_name);
    }
    else
    {
        printf("File download incomplete. Received %ld of %ld bytes.\n", total_bytes_received, file_size);
    }

    fclose(file);
}

/**
 * @brief Handle the group chat functionality, allowing message sending and file operations.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param command The command string representing the user's action.
 */
void handle_chat(int sockfd, char *command)
{
    printf("\n\n\nyou are now in : %s\n", group_name);

    printf("--------------------\nAvailable commands:\n");
    printf("<message> to send a message\n");
    printf("upload_file <file path> to upload a file to group\n");
    printf("download_file <file name> to download file from group\n");
    printf("list_files to list available files in group\n");
    printf("--------------------\n");

    struct pollfd fds[2];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 2, -1);
        if (ret == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN)
        {
            char buffer[BUFFER_SIZE];
            receive_message(sockfd, buffer, sizeof(buffer) - 1, 0);
            printf("%s\n", buffer);
        }

        if (fds[1].revents & POLLIN)
        {
            fgets(command, BUFFER_SIZE, stdin);
            command[strcspn(command, "\n")] = '\0';

            if (strncmp(command, "exit", 4) == 0)
            {
                char exit_command[BUFFER_SIZE];
                snprintf(exit_command, sizeof(exit_command), "message %s %s %d %s", group_name, current_user, 0, command);
                send_message(sockfd, exit_command, strlen(exit_command), 0);
                printf("Leaving group %s...\n", group_name);
                break;
            }
            else if (strncmp(command, "upload_file", 11) == 0)
            {
                char file_path[BUFFER_SIZE];
                sscanf(command + 12, "%s", file_path);

                upload_file(sockfd, group_name, file_path);
            }
            else if (strncmp(command, "download_file", 13) == 0)
            {
                char file_name[BUFFER_SIZE];
                sscanf(command + 14, "%s", file_name);

                download_file(sockfd, group_name, file_name);
            }
            else if (strncmp(command, "list_files", 10) == 0)
            {
                char list_files_command[BUFFER_SIZE];
                snprintf(list_files_command, sizeof(list_files_command), "list_files %s", group_name);
                send_message(sockfd, list_files_command, strlen(list_files_command), 0);

                char buffer[BUFFER_SIZE];
                receive_message(sockfd, buffer, sizeof(buffer) - 1, 0);
                printf("%s\n", buffer);
            }
            else
            {
                char message_command[BUFFER_SIZE];
                snprintf(message_command, sizeof(message_command), "message %s %s %d %s", group_name, current_user, 1, command);

                send_message(sockfd, message_command, strlen(message_command), 0);
            }
        }
    }
}

/**
 * @brief Download a file from the server.
 *
 * @param sockfd The socket descriptor for the connection.
 * @param group_name The name of the group from which to download the file.
 * @param file_name The name of the file to download.
 */
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

        send_message(sockfd, join_command, strlen(join_command), 0);

        char buffer[BUFFER_SIZE];

        receive_message(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (strncmp(buffer, "Joined group successfully", 25) == 0)
        {
            sscanf(command, "join_group %s", group_name);
            handle_chat(sockfd, command);
        }
    }
}

/**
 * @brief Displays a menu for user interaction and handles commands.
 *
 * This function presents a menu of available commands to the user, including login, creating a new user,
 * listing groups, and joining a group.
 *
 * @param sockfd The socket descriptor for the connection to the server.
 * @param command A buffer to store the user's input commands.
 */
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

        if (strlen(command) == 0)
        {
            continue;
        }

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