#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

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

#define BUFFER_SIZE 8192

extern char current_user[50]; /**< Currently logged-in user's name */
extern int is_in_group;       /**< Flag indicating if the user is in a group */
extern char group_name[50];   /**< Name of the group the user is currently in */

void send_command(int sockfd, char *command);
void handle_login_command(int sockfd, char *command);
void upload_file(int sockfd, char *group_name, char *file_path);
void download_file(int sockfd, char *group_name, char *file_name);
void handle_chat(int sockfd, char *command);
void handle_join_command(int sockfd, char *command);
void menu(int sockfd, char *command);

#endif // CLIENT_UTILS_H