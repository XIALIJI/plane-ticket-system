#include "server.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CRITICAL_SECTION g_dataLock;

/*
 * 函数名称：ensureDataDir
 * 函数功能：确保 data 数据目录存在。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：用户注册需要写入 users.txt，因此写文件前先保证数据目录存在。
 */
static void ensureDataDir(void)
{
    (void)_mkdir(DATA_DIR);
}

/*
 * 函数名称：parseUserLine
 * 函数功能：解析用户文件中的一行用户记录。
 * 参数说明：line 为文件中的一行文本；user 为解析后的用户结构体。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：优先解析新版“|”分隔格式，同时兼容旧版“用户名 密码”格式。
 */
static int parseUserLine(const char *line, UserRecord *user)
{
    int ret;

    if (line == 0 || user == 0)
    {
        return 0;
    }

    memset(user, 0, sizeof(UserRecord));
    ret = sscanf_s(line, "%49[^|]|%49[^|]|%d|%19[^|]|%d",
        user->username, (unsigned)_countof(user->username),
        user->password, (unsigned)_countof(user->password),
        &user->points,
        user->level, (unsigned)_countof(user->level),
        &user->isAdmin);

    if (ret == 5)
    {
        return 1;
    }

    ret = sscanf_s(line, "%49s %49s",
        user->username, (unsigned)_countof(user->username),
        user->password, (unsigned)_countof(user->password));
    if (ret == 2)
    {
        user->points = 0;
        copyText(user->level, (int)sizeof(user->level), "普通会员");
        user->isAdmin = strcmp(user->username, "admin") == 0 ? 1 : 0;
        return 1;
    }

    return 0;
}

/*
 * 函数名称：loadUsers
 * 函数功能：从用户文件读取用户列表。
 * 参数说明：users 为用户数组；maxCount 为最多读取的用户数量。
 * 返回值：实际读取到的用户数量。
 * 实现说明：逐行读取 users.txt，解析成功的记录才会加入数组。
 */
static int loadUsers(UserRecord users[], int maxCount)
{
    FILE *fp;
    char line[256];
    int count = 0;

    ensureDataDir();
    if (fopen_s(&fp, USERS_FILE, "r") != 0 || fp == 0)
    {
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != 0)
    {
        UserRecord user;
        if (parseUserLine(line, &user))
        {
            users[count++] = user;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：saveUsers
 * 函数功能：将用户数组保存到用户文件。
 * 参数说明：users 为用户数组；count 为需要保存的用户数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：统一保存用户名、密码、积分、等级和管理员标记。
 */
static int saveUsers(const UserRecord users[], int count)
{
    FILE *fp;
    int i;

    ensureDataDir();
    if (fopen_s(&fp, USERS_FILE, "w") != 0 || fp == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s|%s|%d|%s|%d\n",
            users[i].username,
            users[i].password,
            users[i].points,
            users[i].level,
            users[i].isAdmin);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：handleRegister
 * 函数功能：处理客户端注册请求。
 * 参数说明：request 为注册请求包；response 为服务器响应包。
 * 返回值：注册成功返回 1，失败返回 0。
 * 实现说明：进入临界区后检查用户名是否重复，再追加新用户并保存文件。
 */
int handleRegister(const Packet *request, Packet *response)
{
    UserRecord users[MAX_USERS_COUNT];
    UserRecord newUser;
    char username[50];
    char password[50];
    int count;
    int i;

    EnterCriticalSection(&g_dataLock);
    /* 注册会修改用户文件，因此需要加锁防止多个客户端同时注册同名用户。 */
    count = loadUsers(users, MAX_USERS_COUNT);

    memset(&newUser, 0, sizeof(newUser));
    if (sscanf_s(request->data, "%49[^|]|%49s",
        username, (unsigned)_countof(username),
        password, (unsigned)_countof(password)) != 2)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "注册数据格式错误");
        LeaveCriticalSection(&g_dataLock);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            response->result = RESULT_ERROR;
            copyText(response->data, MAX_DATA, "用户名已存在");
            LeaveCriticalSection(&g_dataLock);
            return 0;
        }
    }

    copyText(newUser.username, (int)sizeof(newUser.username), username);
    copyText(newUser.password, (int)sizeof(newUser.password), password);
    copyText(newUser.level, (int)sizeof(newUser.level), "普通会员");
    newUser.points = 0;
    newUser.isAdmin = 0;

    if (count >= MAX_USERS_COUNT)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "用户数量已达上限");
        LeaveCriticalSection(&g_dataLock);
        return 0;
    }

    users[count++] = newUser;
    response->result = saveUsers(users, count) ? RESULT_OK : RESULT_ERROR;
    copyText(response->data, MAX_DATA, response->result == RESULT_OK ? "注册成功" : "保存用户文件失败");
    LeaveCriticalSection(&g_dataLock);
    return response->result == RESULT_OK;
}

/*
 * 函数名称：handleLogin
 * 函数功能：处理普通用户或管理员登录请求。
 * 参数说明：session 为当前客户端会话；request 为登录请求包；response 为服务器响应包。
 * 返回值：登录成功返回 1，失败返回 0。
 * 实现说明：登录成功后在 Session 中保存用户名、登录状态和管理员身份，后续请求依赖这些会话信息。
 */
int handleLogin(Session *session, const Packet *request, Packet *response)
{
    UserRecord users[MAX_USERS_COUNT];
    char username[50];
    char password[50];
    int count;
    int i;

    count = loadUsers(users, MAX_USERS_COUNT);
    if (sscanf_s(request->data, "%49[^|]|%49s",
        username, (unsigned)_countof(username),
        password, (unsigned)_countof(password)) != 2)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "登录数据格式错误");
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            if (request->cmd == CMD_ADMIN_LOGIN && !users[i].isAdmin)
            {
                /* 管理员入口必须使用管理员账号，普通账号不能进入管理员菜单。 */
                response->result = RESULT_ADMIN_REQUIRED;
                copyText(response->data, MAX_DATA, "该账号不是管理员");
                return 0;
            }

            session->loginState = 1;
            session->isAdmin = users[i].isAdmin;
            copyText(session->username, (int)sizeof(session->username), username);
            copyText(response->username, (int)sizeof(response->username), username);
            response->result = RESULT_OK;
            copyText(response->data, MAX_DATA, users[i].isAdmin ? "管理员登录成功" : "用户登录成功");
            return 1;
        }
    }

    response->result = RESULT_ERROR;
    copyText(response->data, MAX_DATA, "用户名或密码错误");
    return 0;
}

/*
 * 函数名称：handleViewUsers
 * 函数功能：管理员查看所有用户信息。
 * 参数说明：session 为当前客户端会话；response 为服务器响应包。
 * 返回值：查看成功返回 1，权限不足返回 0。
 * 实现说明：只允许管理员访问，结果按多行文本拼接到 Packet.data 返回客户端。
 */
int handleViewUsers(Session *session, Packet *response)
{
    UserRecord users[MAX_USERS_COUNT];
    char line[160];
    int count;
    int i;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    count = loadUsers(users, MAX_USERS_COUNT);
    copyText(response->data, MAX_DATA, "用户名\t积分\t等级\t角色\n");
    for (i = 0; i < count; i++)
    {
        sprintf_s(line, sizeof(line), "%s\t%d\t%s\t%s\n",
            users[i].username, users[i].points, users[i].level, users[i].isAdmin ? "管理员" : "用户");
        appendText(response->data, MAX_DATA, line);
    }

    return 1;
}
