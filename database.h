#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 100
#define MAX_GROUPS 50
#define MAX_GROUP_MEMBERS 10
#define MAX_LINE_LENGTH 256

typedef struct
{
    char username[50];
    char gender;
    int age;
    char password[50];
} User;

typedef struct
{
    char group_name[50];
    char members[MAX_GROUP_MEMBERS][50];
    int member_count;
} Group;

extern User users[MAX_USERS];
extern int user_count;

extern Group groups[MAX_GROUPS];
extern int group_count;

void add_user(const char *username, char gender, int age, const char *password);
void add_group(const char *group_name, char members[MAX_GROUP_MEMBERS][50], int member_count);
void parse_file(const char *filename);
void write_file(const char *filename);

#endif // DATABASE_H