#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>

#define PORT 8081
#define BUFFER_SIZE 8192

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

void upload_file(int sockfd, const char *group_name, const char *file_path)
{
    // Open the file for reading
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    // Extract the file name from the file path
    char *file_name = basename((char *)file_path);

    // Send the command to the server
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "upload_file %s %s", group_name, file_name);
    if (send(sockfd, command, strlen(command), 0) == -1)
    {
        perror("send");
        fclose(file);
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send the file size to the server
    if (send(sockfd, &file_size, sizeof(file_size), 0) == -1)
    {
        perror("send");
        fclose(file);
        return;
    }

    // Read the file and send its contents to the server
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
    printf("File sent successfully\n");
}

void download_file(int sockfd, const char *group_name, const char *file_name)
{
    // Prepare the download command
    char download_command[BUFFER_SIZE];
    snprintf(download_command, sizeof(download_command), "download_file %s %s", group_name, file_name);
    if (send(sockfd, download_command, strlen(download_command), 0) == -1)
    {
        perror("send");
        return;
    }

    // Open the file for writing
    FILE *file = fopen(file_name, "wb");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    // Receive the file size from the server
    uint64_t file_size;
    if (recv(sockfd, &file_size, sizeof(file_size), 0) <= 0)
    {
        perror("recv");
        fclose(file);
        return;
    }

    // Receive the file data from the server
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    uint64_t total_bytes_received = 0;
    while (total_bytes_received < file_size && (bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        // Write the received data to the file
        size_t bytes_written = fwrite(buffer, 1, bytes_received, file);
        if (bytes_written != bytes_received)
        {
            perror("fwrite");
            fclose(file);
            return;
        }
        total_bytes_received += bytes_received;
    }

    if (bytes_received < 0)
    {
        perror("recv");
    }

    fclose(file);
    printf("File downloaded and saved as %s\n", file_name);
}

void *listen_to_server(void *arg)
{
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int nbytes;

    while (1)
    {
        nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes > 0)
        {
            buffer[nbytes] = '\0';
            printf("%s\n", buffer);
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

    return NULL;
}

void handle_chat(int sockfd, char *command)
{

    printf("you are now in : %s\n", group_name);

    pthread_t listen_thread;
    if (pthread_create(&listen_thread, NULL, listen_to_server, &sockfd) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    printf("--------------------\nAvailable commands:\n");
    printf("<message> to send a message\n");
    printf("upload_file <file path> to upload a file to group\n");
    printf("download_file <file name> to download file from group\n");
    printf("list_files to list available files in group\n");
    printf("--------------------\n");

    while (1)
    {

        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "exit", 4) == 0)
        {
            char exit_command[BUFFER_SIZE];
            snprintf(exit_command, sizeof(exit_command), "message %s %s %d %s", group_name, current_user, 0, command);
            send(sockfd, exit_command, strlen(exit_command), 0);
            printf("Leaving group %s...\n", group_name);
            break;
        }
        else if (strncmp(command, "upload_file", 11) == 0)
        {
            char file_path[BUFFER_SIZE];
            sscanf(command + 12, "%s", file_path); // Extract file path from command
            upload_file(sockfd, group_name, file_path);
        }
        else if (strncmp(command, "download_file", 13) == 0)
        {
            char file_name[BUFFER_SIZE];
            sscanf(command + 14, "%s", file_name); // Extract file name from command
            download_file(sockfd, group_name, file_name);
        }
        else if (strncmp(command, "list_files", 10) == 0)
        {
            char list_files_command[BUFFER_SIZE];
            snprintf(list_files_command, sizeof(list_files_command), "list_files %s", group_name);
            send(sockfd, list_files_command, strlen(list_files_command), 0);

            // Receive and print response
            char buffer[BUFFER_SIZE];
            int nbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (nbytes > 0)
            {
                buffer[nbytes] = '\0';
                printf("%s\n", buffer);
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
        else
        {
            char message_command[BUFFER_SIZE];
            snprintf(message_command, sizeof(message_command), "message %s %s %d %s", group_name, current_user, 1, command);
            if (send(sockfd, message_command, strlen(message_command), 0) == -1)
            {
                perror("send");
                break;
            }
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