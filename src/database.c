/**
 * @file database.c
 * @brief Implements the user and group management functions.
 *
 * This file contains the definitions of the functions declared in `database.h`, 
 * including the logic for adding users and groups, reading from a file, and writing to a file.
 */

#include "database.h"

struct client_info clients[MAX_CLIENTS];
int client_count = 0;

/**
 * @var users
 * @brief Array that stores all the users in the system.
 * 
 * The array `users` stores information about all the registered users. 
 * It can contain up to `MAX_USERS` entries.
 */
User users[MAX_USERS]; 
int user_count = 0; /**< The current number of registered users in the system. */

/**
 * @var groups
 * @brief Array that stores all the groups in the system.
 * 
 * The array `groups` holds the data about each group, including its members.
 * It can contain up to `MAX_GROUPS` entries.
 */
Group groups[MAX_GROUPS];
int group_count = 0; /**< The current number of groups in the system. */

/**
 * @brief Adds a new user to the system.
 *
 * This function checks if the maximum number of users (`MAX_USERS`) has been reached.
 * If not, it adds a new user with the specified username, gender, age, and password 
 * to the global `users` array and increments the user count.
 *
 * @param username The username of the new user (maximum length: 50 characters).
 * @param gender The gender of the new user ('M' for male, 'F' for female, etc.).
 * @param age The age of the new user.
 * @param password The password for the new user (maximum length: 50 characters).
 */
void add_user(const char *username, char gender, int age, const char *password)
{
    if (user_count < MAX_USERS) 
    {
        strncpy(users[user_count].username, username, sizeof(users[user_count].username) - 1);
        users[user_count].username[sizeof(users[user_count].username) - 1] = '\0'; // Null-termination
        users[user_count].gender = gender;
        users[user_count].age = age;
        strncpy(users[user_count].password, password, sizeof(users[user_count].password) - 1);
        users[user_count].password[sizeof(users[user_count].password) - 1] = '\0'; // Null-termination
        user_count++;  // Increment the user count
    }
    else
    {
        fprintf(stderr, "Maximum number of users reached.\n");
    }
}

/**
 * @brief Adds a new group to the system.
 *
 * This function checks if the maximum number of groups (`MAX_GROUPS`) has been reached.
 * If not, it adds a new group with the specified group name and members to the global `groups` array
 * and increments the group count.
 *
 * @param group_name The name of the new group (maximum length: 50 characters).
 * @param members Array of members to add to the group (each member's name can have a maximum length of 50 characters).
 * @param member_count The number of members in the group.
 */
void add_group(const char *group_name, char members[MAX_GROUP_MEMBERS][50], int member_count)
{
    if (group_count < MAX_GROUPS) 
    {
        strncpy(groups[group_count].group_name, group_name, sizeof(groups[group_count].group_name) - 1);
        groups[group_count].group_name[sizeof(groups[group_count].group_name) - 1] = '\0'; // Null-termination
        groups[group_count].member_count = member_count;
        
        for (int i = 0; i < member_count; i++) 
        {
            strncpy(groups[group_count].members[i], members[i], sizeof(groups[group_count].members[i]) - 1);
            groups[group_count].members[i][sizeof(groups[group_count].members[i]) - 1] = '\0'; // Null-termination
        }
        group_count++;  // Increment the group count
    }
    else
    {
        fprintf(stderr, "Maximum number of groups reached.\n");
    }
}

/**
 * @brief Parses a file and loads users and groups from it.
 *
 * This function opens the specified file and reads the user and group information line by line.
 * Users are expected to be in the format: `username gender age password`. 
 * Groups are expected to be in the format: `group group_name member1 member2 ...`.
 * 
 * @param filename The name of the file to parse.
 */
void parse_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "group", 5) == 0)
        {
            // Parse group data
            char members[MAX_GROUP_MEMBERS][50];
            int member_count = 0;
            char group_name[50];

            char *token = strtok(line, " \n");
            token = strtok(NULL, " \n"); // Skip "group"
            if (token)
            {
                strncpy(group_name, token, sizeof(group_name) - 1);
                group_name[sizeof(group_name) - 1] = '\0';

                token = strtok(NULL, " \n"); // Get first member
                while (token && member_count < MAX_GROUP_MEMBERS)
                {
                    strncpy(members[member_count], token, sizeof(members[member_count]) - 1);
                    members[member_count][sizeof(members[member_count]) - 1] = '\0';
                    member_count++;
                    token = strtok(NULL, " \n");
                }

                add_group(group_name, members, member_count);
            }
        }
        else
        {
            // Parse user data
            char username[50], password[50], gender;
            int age;

            sscanf(line, "%s %c %d %s", username, &gender, &age, password);
            add_user(username, gender, age, password);
        }
    }

    fclose(file);
}

/**
 * @brief Writes the current users and groups to a file.
 *
 * This function writes all the users and groups from the system to the specified file.
 * Users are written in the format: `username gender age password`. 
 * Groups are written as: `group group_name member1 member2 ...`.
 *
 * @param filename The name of the file to write to.
 */
void write_file(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Write users
    for (int i = 0; i < user_count; i++)
    {
        fprintf(file, "%s %c %d %s\n", users[i].username, (int)users[i].gender, users[i].age, users[i].password);
    }

    // Write groups
    for (int i = 0; i < group_count; i++)
    {
        fprintf(file, "group");
        for (int j = 0; j < groups[i].member_count; j++)
        {
            fprintf(file, " %s", groups[i].members[j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}
