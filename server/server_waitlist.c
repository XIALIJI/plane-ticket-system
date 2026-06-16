#include "server.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CRITICAL_SECTION g_dataLock;

int loadFlightsData(FlightRecord flights[], int maxCount);

/*
 * 函数名称：ensureDataDir
 * 函数功能：确保 data 数据目录存在。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：候补队列文件保存在 data 目录下，保存前需要确保目录可用。
 */
static void ensureDataDir(void)
{
    (void)_mkdir(DATA_DIR);
}

/*
 * 函数名称：parseWaitLine
 * 函数功能：解析候补文件中的一行候补记录。
 * 参数说明：line 为文件原始行；waitItem 为解析后的候补记录。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：候补记录包含用户、乘客、航班、票数和优先级，字段使用“|”分隔。
 */
static int parseWaitLine(const char *line, WaitRecord *waitItem)
{
    if (line == 0 || waitItem == 0)
    {
        return 0;
    }

    memset(waitItem, 0, sizeof(WaitRecord));
    return sscanf_s(line, "%49[^|]|%99[^|]|%29[^|]|%19[^|]|%d|%d",
        waitItem->username, (unsigned)_countof(waitItem->username),
        waitItem->name, (unsigned)_countof(waitItem->name),
        waitItem->phone, (unsigned)_countof(waitItem->phone),
        waitItem->flightNo, (unsigned)_countof(waitItem->flightNo),
        &waitItem->ticketNum,
        &waitItem->priority) == 6;
}

/*
 * 函数名称：loadWaitData
 * 函数功能：从候补文件读取候补队列。
 * 参数说明：waits 为候补数组；maxCount 为最多读取数量。
 * 返回值：实际读取到的候补记录数量。
 * 实现说明：候补记录在内存中使用数组保存，读取时按文件顺序加载。
 */
int loadWaitData(WaitRecord waits[], int maxCount)
{
    FILE *fp;
    char line[512];
    int count = 0;

    ensureDataDir();
    if (fopen_s(&fp, WAITLIST_FILE, "r") != 0 || fp == 0)
    {
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != 0)
    {
        WaitRecord item;
        if (parseWaitLine(line, &item))
        {
            waits[count++] = item;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：saveWaitData
 * 函数功能：保存候补队列到文件。
 * 参数说明：waits 为候补数组；count 为候补记录数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：订票余票不足和候补自动出票后都会刷新该文件。
 */
int saveWaitData(const WaitRecord waits[], int count)
{
    FILE *fp;
    int i;

    ensureDataDir();
    if (fopen_s(&fp, WAITLIST_FILE, "w") != 0 || fp == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s|%s|%s|%s|%d|%d\n",
            waits[i].username,
            waits[i].name,
            waits[i].phone,
            waits[i].flightNo,
            waits[i].ticketNum,
            waits[i].priority);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：sortWaitByPriority
 * 函数功能：按候补优先级从高到低排序。
 * 参数说明：waits 为候补数组；count 为候补数量。
 * 返回值：无。
 * 实现说明：使用冒泡排序，优先级越高越靠前，体现 VIP 候补优先。
 */
static void sortWaitByPriority(WaitRecord waits[], int count)
{
    int i;
    int j;

    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - 1 - i; j++)
        {
            if (waits[j].priority < waits[j + 1].priority)
            {
                WaitRecord temp = waits[j];
                waits[j] = waits[j + 1];
                waits[j + 1] = temp;
            }
        }
    }
}

/*
 * 函数名称：handleWaitlist
 * 函数功能：处理查看候补队列请求。
 * 参数说明：session 为当前客户端会话；request 为请求包；response 为服务器响应包。
 * 返回值：查看成功返回 1，失败返回 0。
 * 实现说明：普通用户只看自己的候补记录，管理员可以查看所有候补记录。
 */
int handleWaitlist(Session *session, const Packet *request, Packet *response)
{
    WaitRecord *waits;
    char line[180];
    int count;
    int i;
    int found = 0;

    (void)request;
    if (!session->loginState)
    {
        response->result = RESULT_LOGIN_REQUIRED;
        copyText(response->data, MAX_DATA, "请先登录");
        return 0;
    }

    waits = (WaitRecord *)malloc(sizeof(WaitRecord) * MAX_WAIT_COUNT);
    if (waits == 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "服务器内存不足，无法查看候补");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    count = loadWaitData(waits, MAX_WAIT_COUNT);
    /* 展示前按优先级排序，便于观察 VIP 或高积分用户的候补顺序。 */
    sortWaitByPriority(waits, count);
    copyText(response->data, MAX_DATA, "用户\t姓名\t航班\t票数\t优先级\n");
    for (i = 0; i < count; i++)
    {
        if (session->isAdmin || strcmp(waits[i].username, session->username) == 0)
        {
            sprintf_s(line, sizeof(line), "%s\t%s\t%s\t%d\t%d\n",
                waits[i].username, waits[i].name, waits[i].flightNo,
                waits[i].ticketNum, waits[i].priority);
            appendText(response->data, MAX_DATA, line);
            found = 1;
        }
    }
    LeaveCriticalSection(&g_dataLock);
    free(waits);

    if (!found)
    {
        copyText(response->data, MAX_DATA, "暂无候补记录");
    }

    return 1;
}

/*
 * 函数名称：handleStatistics
 * 函数功能：管理员查看销售统计和上座率统计。
 * 参数说明：session 为当前客户端会话；response 为服务器响应包。
 * 返回值：统计成功返回 1，权限不足或内存不足返回 0。
 * 实现说明：统计航班座位、订单销售额、售出票数和候补数量，最后计算平均上座率。
 */
int handleStatistics(Session *session, Packet *response)
{
    FlightRecord *flights;
    WaitRecord *waits;
    FILE *fp;
    char line[512];
    OrderRecord order;
    int flightCount;
    int waitCount;
    int orderCount = 0;
    int totalSeats = 0;
    int remainSeats = 0;
    int totalTickets = 0;
    float totalSales = 0.0f;
    int i;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    flights = (FlightRecord *)malloc(sizeof(FlightRecord) * MAX_FLIGHTS_COUNT);
    waits = (WaitRecord *)malloc(sizeof(WaitRecord) * MAX_WAIT_COUNT);
    if (flights == 0 || waits == 0)
    {
        free(flights);
        free(waits);
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "服务器内存不足，无法统计");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    flightCount = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    waitCount = loadWaitData(waits, MAX_WAIT_COUNT);
    for (i = 0; i < flightCount; i++)
    {
        totalSeats += flights[i].totalSeat;
        remainSeats += flights[i].remainSeat;
    }

    if (fopen_s(&fp, ORDERS_FILE, "r") == 0 && fp != 0)
    {
        while (fgets(line, sizeof(line), fp) != 0)
        {
            memset(&order, 0, sizeof(order));
            if (sscanf_s(line, "%29[^|]|%49[^|]|%99[^|]|%29[^|]|%19[^|]|%d|%f|%39[^|]|%19[^|\n]",
                order.orderId, (unsigned)_countof(order.orderId),
                order.username, (unsigned)_countof(order.username),
                order.name, (unsigned)_countof(order.name),
                order.phone, (unsigned)_countof(order.phone),
                order.flightNo, (unsigned)_countof(order.flightNo),
                &order.ticketNum,
                &order.amount,
                order.eticketNo, (unsigned)_countof(order.eticketNo),
                order.status, (unsigned)_countof(order.status)) == 9)
            {
                orderCount++;
                totalTickets += order.ticketNum;
                totalSales += order.amount;
            }
        }
        fclose(fp);
    }

    sprintf_s(response->data, MAX_DATA,
        "航班总数:%d\n订单总数:%d\n销售票数:%d\n候补记录:%d\n销售额:%.2f\n上座率:%.2f%%",
        flightCount,
        orderCount,
        totalTickets,
        waitCount,
        totalSales,
        totalSeats > 0 ? (float)(totalSeats - remainSeats) * 100.0f / (float)totalSeats : 0.0f);
    LeaveCriticalSection(&g_dataLock);
    free(flights);
    free(waits);
    return 1;
}
