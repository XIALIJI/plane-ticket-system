#include "client.h"
#include "client_ui.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

/*
 * 函数名称：connectServer
 * 函数功能：初始化 Winsock，并连接到本机票务服务器。
 * 参数说明：context 为客户端运行上下文，用于保存套接字、登录状态和用户名。
 * 返回值：连接成功返回 1，失败返回 0。
 * 实现说明：客户端启动时只建立一条 TCP 连接，后续所有菜单请求都复用该连接发送 Packet。
 */
int connectServer(ClientContext *context)
{
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    if (context == 0)
    {
        return 0;
    }

    memset(context, 0, sizeof(ClientContext));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup 失败\n");
        return 0;
    }

    context->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (context->sock == INVALID_SOCKET)
    {
        printf("socket 创建失败\n");
        WSACleanup();
        return 0;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (connect(context->sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("连接服务器失败，请先启动服务器 %s:%d\n", SERVER_IP, SERVER_PORT);
        closesocket(context->sock);
        WSACleanup();
        return 0;
    }

    return 1;
}

/*
 * 函数名称：closeClient
 * 函数功能：通知服务器客户端退出，并释放本地 Socket 和 Winsock 资源。
 * 参数说明：context 为客户端运行上下文。
 * 返回值：无。
 * 实现说明：退出前发送 CMD_EXIT，便于服务器线程正常结束并清理会话。
 */
void closeClient(ClientContext *context)
{
    Packet request;
    Packet response;

    if (context == 0)
    {
        return;
    }

    if (context->sock != INVALID_SOCKET)
    {
        initPacket(&request, CMD_EXIT);
        (void)requestServer(context, &request, &response);
        closesocket(context->sock);
        context->sock = INVALID_SOCKET;
    }

    WSACleanup();
}

/*
 * 函数名称：requestServer
 * 函数功能：完成一次“发送请求包、接收响应包”的客户端通信流程。
 * 参数说明：context 为客户端上下文；request 为请求包；response 为服务器响应包。
 * 返回值：通信成功返回 1，发送或接收失败返回 0。
 * 实现说明：每个业务菜单最终都会调用本函数，因此这里是客户端网络通信的统一入口。
 */
int requestServer(ClientContext *context, Packet *request, Packet *response)
{
    if (context == 0 || request == 0 || response == 0)
    {
        return 0;
    }

    copyText(request->username, (int)sizeof(request->username), context->username);
    if (!sendPacket(context->sock, request))
    {
        printf("发送请求失败\n");
        return 0;
    }

    if (!recvPacket(context->sock, response))
    {
        printf("接收响应失败\n");
        return 0;
    }

    return 1;
}

/*
 * 函数名称：main
 * 函数功能：网络客户端程序入口。
 * 参数说明：无。
 * 返回值：程序正常结束返回 0，连接服务器失败返回 1。
 * 实现说明：先设置控制台编码，再连接服务器，最后进入客户端菜单循环。
 */
int main(void)
{
    ClientContext context;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (!connectServer(&context))
    {
        return 1;
    }

    runClientUi(&context);
    closeClient(&context);
    return 0;
}
