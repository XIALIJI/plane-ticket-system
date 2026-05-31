#include "user.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 1000

static int parseUserLine(const char *line, User *user)
{
    if (line == NULL || user == NULL)
    {
        return 0;
    }

#ifdef _MSC_VER
    return sscanf_s(line, "%49s %49s",
                    user->username, (unsigned)_countof(user->username),
                    user->password, (unsigned)_countof(user->password)) == 2;
#else
    return sscanf(line, "%49s %49s", user->username, user->password) == 2;
#endif
}

static int loadUsers(User users[], int maxCount)
{
    FILE *fp;
    char line[256];
    int count = 0;

    if (users == NULL || maxCount <= 0)
    {
        return 0;
    }

    fp = fopen(USER_FILE, "r");
    if (fp == NULL)
    {
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != NULL)
    {
        User user;
        if (parseUserLine(line, &user))
        {
            users[count++] = user;
        }
    }

    fclose(fp);
    return count;
}

static int appendUser(const User *user)
{
    FILE *fp;

    if (user == NULL)
    {
        return 0;
    }

    fp = fopen(USER_FILE, "a");
    if (fp == NULL)
    {
        printf("错误：无法打开 %s 写入。\n", USER_FILE);
        return 0;
    }

    fprintf(fp, "%s %s\n", user->username, user->password);
    fclose(fp);
    return 1;
}

void registerUser(void)
{
    User users[MAX_USERS];
    User newUser;
    int count;
    int i;

    count = loadUsers(users, MAX_USERS);
    readString("请输入用户名：", newUser.username, sizeof(newUser.username));

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, newUser.username) == 0)
        {
            printf("用户名已存在，注册失败。\n");
            return;
        }
    }

    readString("请输入密码：", newUser.password, sizeof(newUser.password));

    if (appendUser(&newUser))
    {
        printf("用户注册成功。\n");
    }
}

int userLogin(void)
{
    User users[MAX_USERS];
    char username[50];
    char password[50];
    int count;
    int i;

    count = loadUsers(users, MAX_USERS);
    readString("请输入用户名：", username, sizeof(username));
    readString("请输入密码：", password, sizeof(password));

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0)
        {
            printf("用户登录成功。\n");
            return 1;
        }
    }

    printf("用户名或密码错误。\n");
    return 0;
}
