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
#include "server_utils.h"

#define PORT 8080

// define server2 ip and port
#define OTHER_SERVER_PORT 8081
#define OTHER_SERVER_IP "127.0.0.1"

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