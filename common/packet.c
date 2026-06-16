#include "packet.h"

#include <string.h>

/*
 * 函数名称：initPacket
 * 函数功能：初始化通信数据包，统一清空字段并设置命令码。
 * 参数说明：packet 为待初始化的数据包指针；cmd 为本次请求或响应的命令码。
 * 返回值：无。
 * 实现说明：先将结构体整体清零，避免残留数据影响网络传输，再写入默认结果状态。
 */
void initPacket(Packet *packet, int cmd)
{
    if (packet == 0)
    {
        return;
    }

    memset(packet, 0, sizeof(Packet));
    packet->cmd = cmd;
    packet->result = RESULT_OK;
}
