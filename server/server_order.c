#include "server.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern CRITICAL_SECTION g_dataLock;

int loadFlightsData(FlightRecord flights[], int maxCount);
int saveFlightsData(const FlightRecord flights[], int count);
int findFlightIndex(const FlightRecord flights[], int count, const char *flightNo);
int loadWaitData(WaitRecord waits[], int maxCount);
int saveWaitData(const WaitRecord waits[], int count);

/*
 * 函数名称：ensureDataDir
 * 函数功能：确保 data 数据目录存在。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：订单文件、用户文件和候补文件都依赖该目录，写文件前先创建目录可提高健壮性。
 */
static void ensureDataDir(void)
{
    (void)_mkdir(DATA_DIR);
}

/*
 * 函数名称：todayText
 * 函数功能：生成当天日期字符串。
 * 参数说明：buffer 为日期输出缓冲区；size 为缓冲区大小。
 * 返回值：无。
 * 实现说明：生成 YYYYMMDD 格式文本，用作订单号中的日期部分。
 */
static void todayText(char *buffer, int size)
{
    time_t now;
    struct tm tmValue;

    time(&now);
    localtime_s(&tmValue, &now);
    sprintf_s(buffer, (size_t)size, "%04d%02d%02d",
        tmValue.tm_year + 1900, tmValue.tm_mon + 1, tmValue.tm_mday);
}

/*
 * 函数名称：parseOrderLine
 * 函数功能：解析订单文件中的单行订单记录。
 * 参数说明：line 为文件中的一行文本；order 为解析后的订单结构体。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：订单字段使用“|”分隔，解析前先清空结构体，避免残留数据。
 */
static int parseOrderLine(const char *line, OrderRecord *order)
{
    if (line == 0 || order == 0)
    {
        return 0;
    }

    memset(order, 0, sizeof(OrderRecord));
    return sscanf_s(line, "%29[^|]|%49[^|]|%99[^|]|%29[^|]|%19[^|]|%d|%f|%39[^|]|%19[^|\n]",
        order->orderId, (unsigned)_countof(order->orderId),
        order->username, (unsigned)_countof(order->username),
        order->name, (unsigned)_countof(order->name),
        order->phone, (unsigned)_countof(order->phone),
        order->flightNo, (unsigned)_countof(order->flightNo),
        &order->ticketNum,
        &order->amount,
        order->eticketNo, (unsigned)_countof(order->eticketNo),
        order->status, (unsigned)_countof(order->status)) == 9;
}

/*
 * 函数名称：loadOrders
 * 函数功能：从订单文件中加载订单数组。
 * 参数说明：orders 为订单数组；maxCount 为最大读取数量。
 * 返回值：返回成功读取的订单数量。
 * 实现说明：逐行读取并调用 parseOrderLine，格式不正确的行不会加入订单数组。
 */
static int loadOrders(OrderRecord orders[], int maxCount)
{
    FILE *fp;
    char line[512];
    int count = 0;

    ensureDataDir();
    if (fopen_s(&fp, ORDERS_FILE, "r") != 0 || fp == 0)
    {
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != 0)
    {
        OrderRecord order;
        if (parseOrderLine(line, &order))
        {
            orders[count++] = order;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：saveOrders
 * 函数功能：将内存中的订单数组保存到订单文件。
 * 参数说明：orders 为订单数组；count 为需要保存的订单数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：订票、退票和候补自动出票后都会调用本函数同步持久化数据。
 */
static int saveOrders(const OrderRecord orders[], int count)
{
    FILE *fp;
    int i;

    ensureDataDir();
    if (fopen_s(&fp, ORDERS_FILE, "w") != 0 || fp == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s|%s|%s|%s|%s|%d|%.2f|%s|%s\n",
            orders[i].orderId,
            orders[i].username,
            orders[i].name,
            orders[i].phone,
            orders[i].flightNo,
            orders[i].ticketNum,
            orders[i].amount,
            orders[i].eticketNo,
            orders[i].status);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：generateOrderNo
 * 函数功能：自动生成唯一订单号。
 * 参数说明：orders 为已有订单数组；count 为订单数量；orderId 为输出缓冲区；orderIdSize 为缓冲区大小。
 * 返回值：无。
 * 实现说明：订单号格式为 OD + 日期 + 四位流水号，通过扫描当天最大流水号实现递增。
 */
static void generateOrderNo(const OrderRecord orders[], int count, char *orderId, int orderIdSize)
{
    char date[16];
    char prefix[24];
    int maxSerial = 0;
    int i;

    todayText(date, sizeof(date));
    sprintf_s(prefix, sizeof(prefix), "OD%s", date);
    for (i = 0; i < count; i++)
    {
        if (strncmp(orders[i].orderId, prefix, strlen(prefix)) == 0)
        {
            int serial = atoi(orders[i].orderId + strlen(prefix));
            if (serial > maxSerial)
            {
                maxSerial = serial;
            }
        }
    }

    sprintf_s(orderId, (size_t)orderIdSize, "%s%04d", prefix, maxSerial + 1);
}

/*
 * 函数名称：generateEticketNo
 * 函数功能：根据订单号生成电子客票号。
 * 参数说明：orderId 为订单号；eticketNo 为电子客票号输出缓冲区；eticketNoSize 为缓冲区大小。
 * 返回值：无。
 * 实现说明：电子客票号采用 ETK-订单号 的形式，便于订单与客票对应。
 */
static void generateEticketNo(const char *orderId, char *eticketNo, int eticketNoSize)
{
    sprintf_s(eticketNo, (size_t)eticketNoSize, "ETK-%s", orderId);
}

/*
 * 函数名称：userPriority
 * 函数功能：计算用户候补优先级。
 * 参数说明：username 为用户名。
 * 返回值：返回候补优先级，数值越大越优先。
 * 实现说明：VIP 用户拥有较高优先级，普通用户根据积分折算优先级。
 */
static int userPriority(const char *username)
{
    FILE *fp;
    char line[256];
    UserRecord user;

    if (fopen_s(&fp, USERS_FILE, "r") != 0 || fp == 0)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), fp) != 0)
    {
        memset(&user, 0, sizeof(user));
        if (sscanf_s(line, "%49[^|]|%49[^|]|%d|%19[^|]|%d",
            user.username, (unsigned)_countof(user.username),
            user.password, (unsigned)_countof(user.password),
            &user.points,
            user.level, (unsigned)_countof(user.level),
            &user.isAdmin) == 5 &&
            strcmp(user.username, username) == 0)
        {
            fclose(fp);
            return strcmp(user.level, "VIP") == 0 ? 100 : user.points / 100;
        }
    }

    fclose(fp);
    return 0;
}

/*
 * 函数名称：addMemberPoints
 * 函数功能：为订票成功的用户增加会员积分。
 * 参数说明：username 为用户名；points 为增加的积分数量。
 * 返回值：无。
 * 实现说明：读取用户文件，找到目标用户后增加积分，并根据积分更新会员等级。
 */
static void addMemberPoints(const char *username, int points)
{
    FILE *fp;
    UserRecord users[MAX_USERS_COUNT];
    char line[256];
    int count = 0;
    int i;

    if (fopen_s(&fp, USERS_FILE, "r") == 0 && fp != 0)
    {
        while (count < MAX_USERS_COUNT && fgets(line, sizeof(line), fp) != 0)
        {
            if (sscanf_s(line, "%49[^|]|%49[^|]|%d|%19[^|]|%d",
                users[count].username, (unsigned)_countof(users[count].username),
                users[count].password, (unsigned)_countof(users[count].password),
                &users[count].points,
                users[count].level, (unsigned)_countof(users[count].level),
                &users[count].isAdmin) == 5)
            {
                count++;
            }
        }
        fclose(fp);
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            users[i].points += points;
            copyText(users[i].level, (int)sizeof(users[i].level), users[i].points >= 1000 ? "VIP" : "普通会员");
            break;
        }
    }

    if (fopen_s(&fp, USERS_FILE, "w") == 0 && fp != 0)
    {
        for (i = 0; i < count; i++)
        {
            fprintf(fp, "%s|%s|%d|%s|%d\n", users[i].username, users[i].password,
                users[i].points, users[i].level, users[i].isAdmin);
        }
        fclose(fp);
    }
}

/*
 * 函数名称：autoBookFromWaitlist
 * 函数功能：退票后自动处理候补队列出票。
 * 参数说明：flights 为航班数组；flightCount 为航班数量；orders 为订单数组；orderCount 为订单数量指针。
 * 返回值：返回自动出票成功的候补记录数量。
 * 实现说明：遍历候补记录，若对应航班余票足够，则生成订单、扣减余票并从候补队列删除该记录。
 */
static int autoBookFromWaitlist(FlightRecord flights[], int flightCount, OrderRecord orders[], int *orderCount)
{
    WaitRecord *waits;
    int waitCount;
    int i;
    int booked = 0;

    waits = (WaitRecord *)malloc(sizeof(WaitRecord) * MAX_WAIT_COUNT);
    if (waits == 0)
    {
        return 0;
    }

    waitCount = loadWaitData(waits, MAX_WAIT_COUNT);
    for (i = 0; i < waitCount; )
    {
        int flightIndex = findFlightIndex(flights, flightCount, waits[i].flightNo);
        if (flightIndex >= 0 && flights[flightIndex].remainSeat >= waits[i].ticketNum && *orderCount < MAX_ORDERS_COUNT)
        {
            OrderRecord order;
            int j;

            /* 候补记录满足出票条件时，按正常订票流程生成订单和电子客票。 */
            memset(&order, 0, sizeof(order));
            generateOrderNo(orders, *orderCount, order.orderId, sizeof(order.orderId));
            copyText(order.username, (int)sizeof(order.username), waits[i].username);
            copyText(order.name, (int)sizeof(order.name), waits[i].name);
            copyText(order.phone, (int)sizeof(order.phone), waits[i].phone);
            copyText(order.flightNo, (int)sizeof(order.flightNo), waits[i].flightNo);
            order.ticketNum = waits[i].ticketNum;
            order.amount = flights[flightIndex].price * order.ticketNum;
            generateEticketNo(order.orderId, order.eticketNo, sizeof(order.eticketNo));
            copyText(order.status, (int)sizeof(order.status), "已出票");

            flights[flightIndex].remainSeat -= order.ticketNum;
            orders[(*orderCount)++] = order;
            addMemberPoints(order.username, (int)order.amount / 10);

            /* 已经出票的候补记录需要从数组中删除，后续元素整体前移。 */
            for (j = i; j < waitCount - 1; j++)
            {
                waits[j] = waits[j + 1];
            }
            waitCount--;
            booked++;
        }
        else
        {
            i++;
        }
    }

    saveWaitData(waits, waitCount);
    free(waits);
    return booked;
}

/*
 * 函数名称：handleBook
 * 函数功能：处理用户订票请求。
 * 参数说明：session 为当前客户端会话；request 为订票请求包；response 为服务器响应包。
 * 返回值：订票成功或加入候补成功返回 1，失败返回 0。
 * 实现说明：订票涉及余票、订单和候补三类文件，使用临界区保证并发情况下不会超卖。
 */
int handleBook(Session *session, const Packet *request, Packet *response)
{
    FlightRecord *flights;
    OrderRecord *orders;
    WaitRecord *waits;
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;
    int flightCount;
    int orderCount;
    int waitCount;
    int flightIndex;

    if (!session->loginState)
    {
        response->result = RESULT_LOGIN_REQUIRED;
        copyText(response->data, MAX_DATA, "请先登录");
        return 0;
    }

    if (sscanf_s(request->data, "%99[^|]|%29[^|]|%19[^|]|%d",
        name, (unsigned)_countof(name),
        phone, (unsigned)_countof(phone),
        flightNo, (unsigned)_countof(flightNo),
        &ticketNum) != 4 || ticketNum <= 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "订票数据格式错误");
        return 0;
    }

    flights = (FlightRecord *)malloc(sizeof(FlightRecord) * MAX_FLIGHTS_COUNT);
    orders = (OrderRecord *)malloc(sizeof(OrderRecord) * MAX_ORDERS_COUNT);
    waits = (WaitRecord *)malloc(sizeof(WaitRecord) * MAX_WAIT_COUNT);
    if (flights == 0 || orders == 0 || waits == 0)
    {
        free(flights);
        free(orders);
        free(waits);
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "服务器内存不足，订票失败");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    /* 进入临界区后再读取、检查和保存，防止多个客户端同时修改余票。 */
    flightCount = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    orderCount = loadOrders(orders, MAX_ORDERS_COUNT);
    waitCount = loadWaitData(waits, MAX_WAIT_COUNT);
    flightIndex = findFlightIndex(flights, flightCount, flightNo);
    if (flightIndex < 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班不存在");
        LeaveCriticalSection(&g_dataLock);
        free(flights);
        free(orders);
        free(waits);
        return 0;
    }

    if (flights[flightIndex].remainSeat < ticketNum)
    {
        WaitRecord waitItem;
        if (waitCount >= MAX_WAIT_COUNT)
        {
            response->result = RESULT_ERROR;
            copyText(response->data, MAX_DATA, "候补队列已满");
            LeaveCriticalSection(&g_dataLock);
            free(flights);
            free(orders);
            free(waits);
            return 0;
        }

        memset(&waitItem, 0, sizeof(waitItem));
        copyText(waitItem.username, (int)sizeof(waitItem.username), session->username);
        copyText(waitItem.name, (int)sizeof(waitItem.name), name);
        copyText(waitItem.phone, (int)sizeof(waitItem.phone), phone);
        copyText(waitItem.flightNo, (int)sizeof(waitItem.flightNo), flightNo);
        waitItem.ticketNum = ticketNum;
        waitItem.priority = userPriority(session->username);
        waits[waitCount++] = waitItem;
        /* 余票不足时不创建订单，而是保存为候补记录，等待退票释放座位。 */
        saveWaitData(waits, waitCount);
        response->result = RESULT_OK;
        copyText(response->data, MAX_DATA, "余票不足，已加入候补队列");
        LeaveCriticalSection(&g_dataLock);
        free(flights);
        free(orders);
        free(waits);
        return 1;
    }

    if (orderCount >= MAX_ORDERS_COUNT)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "订单数量已达上限");
        LeaveCriticalSection(&g_dataLock);
        free(flights);
        free(orders);
        free(waits);
        return 0;
    }

    memset(&orders[orderCount], 0, sizeof(OrderRecord));
    /* 余票充足时生成正式订单，订单号和电子客票号由服务器统一生成。 */
    generateOrderNo(orders, orderCount, orders[orderCount].orderId, sizeof(orders[orderCount].orderId));
    copyText(orders[orderCount].username, (int)sizeof(orders[orderCount].username), session->username);
    copyText(orders[orderCount].name, (int)sizeof(orders[orderCount].name), name);
    copyText(orders[orderCount].phone, (int)sizeof(orders[orderCount].phone), phone);
    copyText(orders[orderCount].flightNo, (int)sizeof(orders[orderCount].flightNo), flightNo);
    orders[orderCount].ticketNum = ticketNum;
    orders[orderCount].amount = flights[flightIndex].price * ticketNum;
    generateEticketNo(orders[orderCount].orderId, orders[orderCount].eticketNo, sizeof(orders[orderCount].eticketNo));
    copyText(orders[orderCount].status, (int)sizeof(orders[orderCount].status), "已出票");

    flights[flightIndex].remainSeat -= ticketNum;
    /* 订票成功后增加会员积分，积分会影响后续候补优先级。 */
    addMemberPoints(session->username, (int)orders[orderCount].amount / 10);
    sprintf_s(response->data, MAX_DATA, "订票成功\n订单号:%s\n电子客票:%s\n金额:%.2f",
        orders[orderCount].orderId, orders[orderCount].eticketNo, orders[orderCount].amount);
    orderCount++;

    if (!saveFlightsData(flights, flightCount) || !saveOrders(orders, orderCount))
    {
        /* 任一文件保存失败都视为订票失败，避免客户端误认为订单已经可靠落盘。 */
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "保存订单或航班失败");
        LeaveCriticalSection(&g_dataLock);
        free(flights);
        free(orders);
        free(waits);
        return 0;
    }

    LeaveCriticalSection(&g_dataLock);
    free(flights);
    free(orders);
    free(waits);
    return 1;
}

/*
 * 函数名称：handleCancel
 * 函数功能：处理用户退票请求。
 * 参数说明：session 为当前客户端会话；request->data 为订单号；response 为服务器响应包。
 * 返回值：退票成功返回 1，失败返回 0。
 * 实现说明：退票后恢复航班余票，删除订单，并立即触发候补队列自动出票。
 */
int handleCancel(Session *session, const Packet *request, Packet *response)
{
    FlightRecord *flights;
    OrderRecord *orders;
    int flightCount;
    int orderCount;
    int orderIndex = -1;
    int flightIndex;
    int i;
    int booked;

    if (!session->loginState)
    {
        response->result = RESULT_LOGIN_REQUIRED;
        copyText(response->data, MAX_DATA, "请先登录");
        return 0;
    }

    flights = (FlightRecord *)malloc(sizeof(FlightRecord) * MAX_FLIGHTS_COUNT);
    orders = (OrderRecord *)malloc(sizeof(OrderRecord) * MAX_ORDERS_COUNT);
    if (flights == 0 || orders == 0)
    {
        free(flights);
        free(orders);
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "服务器内存不足，退票失败");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    /* 退票释放座位和候补自动出票必须作为一个整体处理，避免座位被重复使用。 */
    flightCount = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    orderCount = loadOrders(orders, MAX_ORDERS_COUNT);
    for (i = 0; i < orderCount; i++)
    {
        if (strcmp(orders[i].orderId, request->data) == 0 &&
            (session->isAdmin || strcmp(orders[i].username, session->username) == 0))
        {
            orderIndex = i;
            break;
        }
    }

    if (orderIndex < 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "未找到可退订单");
        LeaveCriticalSection(&g_dataLock);
        free(flights);
        free(orders);
        return 0;
    }

    flightIndex = findFlightIndex(flights, flightCount, orders[orderIndex].flightNo);
    if (flightIndex >= 0)
    {
        /* 恢复退票数量对应的余票，并防止余票超过总座位数。 */
        flights[flightIndex].remainSeat += orders[orderIndex].ticketNum;
        if (flights[flightIndex].remainSeat > flights[flightIndex].totalSeat)
        {
            flights[flightIndex].remainSeat = flights[flightIndex].totalSeat;
        }
    }

    for (i = orderIndex; i < orderCount - 1; i++)
    {
        /* 删除订单采用数组前移覆盖方式，保持订单文件中不再出现该订单。 */
        orders[i] = orders[i + 1];
    }
    orderCount--;
    booked = autoBookFromWaitlist(flights, flightCount, orders, &orderCount);

    /* 所有内存变更完成后统一保存，保证航班和订单文件状态一致。 */
    saveFlightsData(flights, flightCount);
    saveOrders(orders, orderCount);
    sprintf_s(response->data, MAX_DATA, "退票成功，自动候补出票 %d 单", booked);
    LeaveCriticalSection(&g_dataLock);
    free(flights);
    free(orders);
    return 1;
}

/*
 * 函数名称：handleOrder
 * 函数功能：查询订单信息。
 * 参数说明：session 为当前客户端会话；request 为请求包；response 为服务器响应包。
 * 返回值：查询处理成功返回 1，未登录或内存不足返回 0。
 * 实现说明：普通用户只能查看自己的订单，管理员会话可以查看全部订单。
 */
int handleOrder(Session *session, const Packet *request, Packet *response)
{
    OrderRecord *orders;
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

    orders = (OrderRecord *)malloc(sizeof(OrderRecord) * MAX_ORDERS_COUNT);
    if (orders == 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "服务器内存不足，无法查看订单");
        return 0;
    }

    count = loadOrders(orders, MAX_ORDERS_COUNT);
    copyText(response->data, MAX_DATA, "订单号\t航班\t票数\t金额\t电子客票\t状态\n");
    for (i = 0; i < count; i++)
    {
        /* 通过会话身份控制订单可见范围，避免普通用户查看他人订单。 */
        if (session->isAdmin || strcmp(orders[i].username, session->username) == 0)
        {
            sprintf_s(line, sizeof(line), "%s\t%s\t%d\t%.2f\t%s\t%s\n",
                orders[i].orderId, orders[i].flightNo, orders[i].ticketNum,
                orders[i].amount, orders[i].eticketNo, orders[i].status);
            appendText(response->data, MAX_DATA, line);
            found = 1;
        }
    }

    if (!found)
    {
        copyText(response->data, MAX_DATA, "暂无订单");
    }

    free(orders);
    return 1;
}

/*
 * 函数名称：handleViewAllOrders
 * 函数功能：管理员查看所有订单。
 * 参数说明：session 为当前客户端会话；response 为服务器响应包。
 * 返回值：查看成功返回 1，权限不足返回 0。
 * 实现说明：先校验管理员权限，再复用 handleOrder 完成订单列表拼接。
 */
int handleViewAllOrders(Session *session, Packet *response)
{
    Packet request;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    initPacket(&request, CMD_VIEW_ALL_ORDERS);
    return handleOrder(session, &request, response);
}
