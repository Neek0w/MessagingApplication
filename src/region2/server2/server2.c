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

#define PORT 8081

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