/**
 * @file database.h
 * @brief Header file defining the user and group structures, as well as functions for managing users and groups.
 *
 * This file contains the structures and function declarations required to manage users, groups,
 * and file I/O operations for a simple database system.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_USERS 100        /**< Maximum number of users in the system */
#define MAX_GROUPS 50        /**< Maximum number of groups in the system */
#define MAX_GROUP_MEMBERS 10 /**< Maximum number of members in a group */
#define MAX_LINE_LENGTH 256  /**< Maximum length of a line in the input file */
#define MAX_CLIENTS 100      /**< Maximum number of clients that can connect to the server */


/**
 * @struct client_info
 * @brief Represents information about a client.
 *
 * The client_info structure stores information about a client, including their username and file descriptor.
 */
struct client_info
{
    char username[50];
    int fd;
};

extern struct client_info clients[MAX_CLIENTS]; /**< Array storing information about all clients */
extern int client_count;                        /**< The current count of clients */

/**
 * @struct User
 * @brief Represents a user in the system.
 *
 * The User structure stores information about a user, including their username, gender, age, and password.
 */
typedef struct
{
    char username[50]; /**< The username of the user */
    char gender;       /**< The gender of the user ('M' for male, 'F' for female, etc.) */
    int age;           /**< The age of the user */
    char password[50]; /**< The password of the user */
} User;

/**
 * @struct Group
 * @brief Represents a group in the system.
 *
 * The Group structure stores the group name and the list of members in the group.
 */
typedef struct
{
    char group_name[50];                 /**< The name of the group */
    char members[MAX_GROUP_MEMBERS][50]; /**< The list of group members */
    int member_count;                    /**< The number of members in the group */
} Group;

extern User users[MAX_USERS]; /**< Array storing all users in the system */
extern int user_count;        /**< The current count of users */

extern Group groups[MAX_GROUPS]; /**< Array storing all groups in the system */
extern int group_count;          /**< The current count of groups */

/**
 * @brief Adds a new user to the system.
 *
 * This function adds a new user with the given username, gender, age, and password.
 *
 * @param username The username of the new user.
 * @param gender The gender of the new user.
 * @param age The age of the new user.
 * @param password The password for the new user.
 */
void add_user(const char *username, char gender, int age, const char *password);

/**
 * @brief Adds a new group to the system.
 *
 * This function adds a new group with the given group name and members.
 *
 * @param group_name The name of the new group.
 * @param members Array of members to add to the group.
 * @param member_count The number of members in the group.
 */
void add_group(const char *group_name, char members[MAX_GROUP_MEMBERS][50], int member_count);

/**
 * @brief Parses a file and loads users and groups.
 *
 * This function reads a file and loads user and group data into the system. The file should contain
 * user and group information in a specific format.
 *
 * @param filename The name of the file to parse.
 */
void parse_file(const char *filename);

/**
 * @brief Writes users and groups to a file.
 *
 * This function writes the current users and groups to a file in a specific format.
 *
 * @param filename The name of the file to write to.
 */
void write_file(const char *filename);

#endif // DATABASE_H
