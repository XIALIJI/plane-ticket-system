#include "server.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

CRITICAL_SECTION g_dataLock;

/*
 * 函数名称：serverInitLocks
 * 函数功能：初始化服务端全局临界区。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：临界区用于保护航班余票、订单和候补文件，避免多客户端并发写入造成数据不一致。
 */
void serverInitLocks(void)
{
    InitializeCriticalSection(&g_dataLock);
}

/*
 * 函数名称：serverDeleteLocks
 * 函数功能：释放服务端全局临界区资源。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：服务器退出前调用，和 InitializeCriticalSection 成对使用。
 */
void serverDeleteLocks(void)
{
    DeleteCriticalSection(&g_dataLock);
}

/*
 * 函数名称：processPacket
 * 函数功能：服务端统一业务分发入口。
 * 参数说明：session 为当前客户端会话；request 为客户端请求包；response 为服务器响应包。
 * 返回值：无。
 * 实现说明：根据 Packet.cmd 命令码调用对应处理函数，所有客户端请求都经过该函数分发。
 */
void processPacket(Session *session, const Packet *request, Packet *response)
{
    initPacket(response, request->cmd);
    copyText(response->username, (int)sizeof(response->username), session->username);

    /* 根据命令码分发业务，保证网络层和具体业务处理解耦。 */
    switch (request->cmd)
    {
    case CMD_REGISTER:
        handleRegister(request, response);
        break;
    case CMD_LOGIN:
    case CMD_ADMIN_LOGIN:
        handleLogin(session, request, response);
        break;
    case CMD_QUERY_FLIGHT:
        handleQueryFlight(request, response);
        break;
    case CMD_ADD_FLIGHT:
        handleAddFlight(session, request, response);
        break;
    case CMD_DELETE_FLIGHT:
        handleDeleteFlight(session, request, response);
        break;
    case CMD_UPDATE_FLIGHT:
        handleUpdateFlight(session, request, response);
        break;
    case CMD_BOOK:
        handleBook(session, request, response);
        break;
    case CMD_CANCEL:
        handleCancel(session, request, response);
        break;
    case CMD_VIEW_ORDER:
        handleOrder(session, request, response);
        break;
    case CMD_WAITLIST:
        handleWaitlist(session, request, response);
        break;
    case CMD_VIEW_USERS:
        handleViewUsers(session, response);
        break;
    case CMD_VIEW_ALL_ORDERS:
        handleViewAllOrders(session, response);
        break;
    case CMD_STATISTICS:
        handleStatistics(session, response);
        break;
    case CMD_EXIT:
        response->result = RESULT_OK;
        copyText(response->data, MAX_DATA, "已退出");
        break;
    default:
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "未知命令");
        break;
    }
}

/*
 * 函数名称：ClientThread
 * 函数功能：处理单个客户端连接的线程函数。
 * 参数说明：lpParam 为 accept 得到的客户端 SOCKET。
 * 返回值：线程结束返回 0。
 * 实现说明：每个客户端连接创建一个独立线程，线程内部维护自己的 Session 登录状态。
 */
DWORD WINAPI ClientThread(LPVOID lpParam)
{
    Session session;
    Packet request;
    Packet response;

    memset(&session, 0, sizeof(session));
    session.sock = (SOCKET)lpParam;

    printf("客户端已连接: socket=%llu\n", (unsigned long long)session.sock);
    while (recvPacket(session.sock, &request))
    {
        /* 收到完整请求包后立即进入业务分发，并把处理结果写入 response。 */
        processPacket(&session, &request, &response);
        if (!sendPacket(session.sock, &response))
        {
            break;
        }

        if (request.cmd == CMD_EXIT)
        {
            break;
        }
    }

    printf("客户端断开: user=%s socket=%llu\n",
        session.loginState ? session.username : "未登录",
        (unsigned long long)session.sock);
    closesocket(session.sock);
    return 0;
}

/*
 * 函数名称：main
 * 函数功能：航空票务服务器程序入口。
 * 参数说明：无。
 * 返回值：启动成功后进入监听循环；启动失败返回 1。
 * 实现说明：按 WSAStartup、socket、bind、listen、accept 的标准 TCP 服务端流程运行。
 */
int main(void)
{
    WSADATA wsaData;
    SOCKET listenSock;
    struct sockaddr_in serverAddr;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup 失败\n");
        return 1;
    }

    serverInitLocks();
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET)
    {
        printf("socket 创建失败\n");
        serverDeleteLocks();
        WSACleanup();
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(listenSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("bind 失败，端口可能被占用: %d\n", SERVER_PORT);
        closesocket(listenSock);
        serverDeleteLocks();
        WSACleanup();
        return 1;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen 失败\n");
        closesocket(listenSock);
        serverDeleteLocks();
        WSACleanup();
        return 1;
    }

    printf("航空票务服务器启动: %s:%d\n", SERVER_IP, SERVER_PORT);
    while (1)
    {
        SOCKET clientSock = accept(listenSock, 0, 0);
        HANDLE threadHandle;

        if (clientSock == INVALID_SOCKET)
        {
            continue;
        }

        /* 每接入一个客户端就创建一个线程，实现多客户端并发访问。 */
        threadHandle = CreateThread(0, 0, ClientThread, (LPVOID)clientSock, 0, 0);
        if (threadHandle == 0)
        {
            closesocket(clientSock);
        }
        else
        {
            CloseHandle(threadHandle);
        }
    }

    closesocket(listenSock);
    serverDeleteLocks();
    WSACleanup();
    return 0;
}
