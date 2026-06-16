#include "protocol.h"

#include <stdio.h>
#include <string.h>

/*
 * 函数名称：sendAll
 * 函数功能：循环发送指定长度的数据，保证一个 Packet 能完整发送出去。
 * 参数说明：sock 为客户端或服务端套接字；buffer 为待发送缓冲区；length 为发送字节数。
 * 返回值：发送完整返回 1，发送失败或连接断开返回 0。
 * 实现说明：TCP 的 send 不保证一次发完全部数据，因此用循环累加已发送字节数。
 */
static int sendAll(SOCKET sock, const char *buffer, int length)
{
    int sent = 0;

    while (sent < length)
    {
        int ret = send(sock, buffer + sent, length - sent, 0);
        if (ret <= 0)
        {
            return 0;
        }
        sent += ret;
    }

    return 1;
}

/*
 * 函数名称：recvAll
 * 函数功能：循环接收指定长度的数据，保证一个 Packet 能完整接收。
 * 参数说明：sock 为通信套接字；buffer 为接收缓冲区；length 为需要接收的字节数。
 * 返回值：接收完整返回 1，接收失败或连接断开返回 0。
 * 实现说明：TCP 是流式协议，一次 recv 可能只收到部分数据，所以必须按长度循环读取。
 */
static int recvAll(SOCKET sock, char *buffer, int length)
{
    int received = 0;

    while (received < length)
    {
        int ret = recv(sock, buffer + received, length - received, 0);
        if (ret <= 0)
        {
            return 0;
        }
        received += ret;
    }

    return 1;
}

/*
 * 函数名称：sendPacket
 * 函数功能：向对端发送一个完整的 Packet 数据包。
 * 参数说明：sock 为通信套接字；packet 为待发送的数据包。
 * 返回值：发送成功返回 1，失败返回 0。
 * 实现说明：本系统采用定长结构体协议，直接按 Packet 的字节长度发送。
 */
int sendPacket(SOCKET sock, const Packet *packet)
{
    if (packet == 0)
    {
        return 0;
    }

    return sendAll(sock, (const char *)packet, (int)sizeof(Packet));
}

/*
 * 函数名称：recvPacket
 * 函数功能：从对端接收一个完整的 Packet 数据包。
 * 参数说明：sock 为通信套接字；packet 为接收结果保存位置。
 * 返回值：接收成功返回 1，失败返回 0。
 * 实现说明：接收前先清空结构体，防止接收失败时残留旧数据。
 */
int recvPacket(SOCKET sock, Packet *packet)
{
    if (packet == 0)
    {
        return 0;
    }

    memset(packet, 0, sizeof(Packet));
    return recvAll(sock, (char *)packet, (int)sizeof(Packet));
}

/*
 * 函数名称：copyText
 * 函数功能：安全复制字符串，防止目标数组越界。
 * 参数说明：dest 为目标缓冲区；destSize 为目标缓冲区大小；src 为源字符串。
 * 返回值：无。
 * 实现说明：兼容 MSVC 和普通 C 编译环境，始终保证目标字符串以 '\0' 结束。
 */
void copyText(char *dest, int destSize, const char *src)
{
    if (dest == 0 || destSize <= 0)
    {
        return;
    }

    if (src == 0)
    {
        dest[0] = '\0';
        return;
    }

#ifdef _MSC_VER
    strncpy_s(dest, (size_t)destSize, src, _TRUNCATE);
#else
    strncpy(dest, src, (size_t)destSize - 1);
    dest[destSize - 1] = '\0';
#endif
}

/*
 * 函数名称：appendText
 * 函数功能：向已有字符串尾部追加内容，并限制最大长度。
 * 参数说明：dest 为目标字符串缓冲区；destSize 为缓冲区大小；src 为追加内容。
 * 返回值：无。
 * 实现说明：用于把多行查询结果拼接到 Packet.data 中，避免超过 MAX_DATA。
 */
void appendText(char *dest, int destSize, const char *src)
{
    int used;

    if (dest == 0 || src == 0 || destSize <= 0)
    {
        return;
    }

    used = (int)strlen(dest);
    if (used >= destSize - 1)
    {
        return;
    }

#ifdef _MSC_VER
    strncat_s(dest, (size_t)destSize, src, _TRUNCATE);
#else
    strncat(dest, src, (size_t)(destSize - used - 1));
#endif
}
