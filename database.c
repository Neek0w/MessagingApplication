#include "database.h"

User users[MAX_USERS];
int user_count = 0;

Group groups[MAX_GROUPS];
int group_count = 0;

void add_user(const char *username, char gender, int age, const char *password)
{
    if (user_count < MAX_USERS)
    {
        strncpy(users[user_count].username, username, sizeof(users[user_count].username) - 1);
        users[user_count].username[sizeof(users[user_count].username) - 1] = '\0'; // Ensure null-termination
        users[user_count].gender = gender;
        users[user_count].age = age;
        strncpy(users[user_count].password, password, sizeof(users[user_count].password) - 1);
        users[user_count].password[sizeof(users[user_count].password) - 1] = '\0'; // Ensure null-termination
        user_count++;
    }
    else
    {
        fprintf(stderr, "Maximum number of users reached.\n");
    }
}

void add_group(const char *group_name, char members[MAX_GROUP_MEMBERS][50], int member_count)
{
    if (group_count < MAX_GROUPS)
    {
        strncpy(groups[group_count].group_name, group_name, sizeof(groups[group_count].group_name) - 1);
        groups[group_count].group_name[sizeof(groups[group_count].group_name) - 1] = '\0'; // Ensure null-termination
        groups[group_count].member_count = member_count;

        // Initialize members to empty strings
        for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
        {
            groups[group_count].members[i][0] = '\0';
        }

        // Remove the loop that copies the actual member names

        group_count++;
    }
    else
    {
        fprintf(stderr, "Maximum number of groups reached.\n");
    }
}

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
            char members[MAX_GROUP_MEMBERS][50];
            int member_count = 0;
            char group_name[50];

            char *token = strtok(line, " \n");
            token = strtok(NULL, " \n"); // Skip "group"
            if (token)
            {
                strncpy(group_name, token, sizeof(group_name) - 1);
                group_name[sizeof(group_name) - 1] = '\0'; // Ensure null-termination

                token = strtok(NULL, " \n"); // Get first member
                while (token && member_count < MAX_GROUP_MEMBERS)
                {
                    strncpy(members[member_count], token, sizeof(members[member_count]) - 1);
                    members[member_count][sizeof(members[member_count]) - 1] = '\0'; // Ensure null-termination
                    member_count++;
                    token = strtok(NULL, " \n");
                }

                add_group(group_name, members, member_count);
            }
        }
        else
        {
            char username[50], password[50], gender;
            int age;

            sscanf(line, "%s %c %d %s", username, &gender, &age, password);
            add_user(username, gender, age, password);
        }
    }

    fclose(file);
}

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