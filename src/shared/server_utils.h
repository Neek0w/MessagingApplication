/**
 * @file server_utils.h
 * @brief Header file containing utility functions for server operations.
 * 
 * Detailed description of the file's purpose and contents.
 */

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

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

#define MAX_CLIENTS 100
#define BUFFER_SIZE 8192

#define OTHER_SERVER_FD 4

void add_client(const char *username, int fd);
void remove_client(int fd);
void handle_login(int client_fd, char *username, char *password);
void handle_create_user(int client_fd, char *username, char *gender, int age, char *password);
void handle_upload_file(int client_fd, const char *group_name, const char *file_name);
void handle_download_file(int client_fd, const char *group_name, const char *file_name);
void handle_list_files(int client_fd, const char *group_name);
void handle_list_groups(int client_fd);
void handle_join_group(int client_fd, char *username, char *group_name);
int get_client_fd_by_username(const char *username);
void handle_message(int client_fd, char *group, char *user, char *message, int type);
void handle_client(int client_fd);
void print_data();

#endif // SERVER_UTILS_H