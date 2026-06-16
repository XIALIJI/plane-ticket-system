#include "user.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 1000

/*
 * 函数名称：parseUserLine
 * 函数功能：解析用户文件中的一行账号记录。
 * 参数说明：line 为文件行文本；user 为解析后的用户结构体。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：用户名和密码以空格分隔保存，解析时限制字段长度防止溢出。
 */
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

/*
 * 函数名称：loadUsers
 * 函数功能：从用户文件加载账号列表。
 * 参数说明：users 为用户数组；maxCount 为最大读取数量。
 * 返回值：返回实际加载的用户数量。
 * 实现说明：注册和登录前都会读取用户文件，保证使用最新账号数据。
 */
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

/*
 * 函数名称：appendUser
 * 函数功能：向用户文件追加一个新账号。
 * 参数说明：user 为待注册用户。
 * 返回值：追加成功返回 1，失败返回 0。
 * 实现说明：注册成功后直接追加写入，减少文件重写开销。
 */
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
        printf("閿欒锛氭棤娉曟墦寮€ %s 鍐欏叆銆俓n", USER_FILE);
        return 0;
    }

    fprintf(fp, "%s %s\n", user->username, user->password);
    fclose(fp);
    return 1;
}

/*
 * 函数名称：registerUser
 * 函数功能：处理普通用户注册。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：先检查用户名是否重复，再保存新用户账号和密码。
 */
void registerUser(void)
{
    User users[MAX_USERS];
    User newUser;
    int count;
    int i;

    count = loadUsers(users, MAX_USERS);
    readString("璇疯緭鍏ョ敤鎴峰悕锛?, newUser.username, sizeof(newUser.username));

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, newUser.username) == 0)
        {
            printf("鐢ㄦ埛鍚嶅凡瀛樺湪锛屾敞鍐屽け璐ャ€俓n");
            return;
        }
    }

    readString("璇疯緭鍏ュ瘑鐮侊細", newUser.password, sizeof(newUser.password));

    if (appendUser(&newUser))
    {
        printf("鐢ㄦ埛娉ㄥ唽鎴愬姛銆俓n");
    }
}

/*
 * 函数名称：userLogin
 * 函数功能：验证普通用户登录。
 * 参数说明：无。
 * 返回值：登录成功返回 1，失败返回 0。
 * 实现说明：遍历用户文件中的账号，用户名和密码同时匹配才允许登录。
 */
int userLogin(void)
{
    User users[MAX_USERS];
    char username[50];
    char password[50];
    int count;
    int i;

    count = loadUsers(users, MAX_USERS);
    readString("璇疯緭鍏ョ敤鎴峰悕锛?, username, sizeof(username));
    readString("璇疯緭鍏ュ瘑鐮侊細", password, sizeof(password));

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0)
        {
            printf("鐢ㄦ埛鐧诲綍鎴愬姛銆俓n");
            return 1;
        }
    }

    printf("鐢ㄦ埛鍚嶆垨瀵嗙爜閿欒銆俓n");
    return 0;
}
