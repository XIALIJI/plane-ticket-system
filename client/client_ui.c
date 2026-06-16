#include "client_ui.h"
#include "../common/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 函数名称：trimNewline
 * 函数功能：去掉 fgets 读入字符串末尾的换行符。
 * 参数说明：text 为需要处理的字符串。
 * 返回值：无。
 * 实现说明：循环删除 '\n' 和 '\r'，兼容 Windows 文本输入中的回车换行。
 */
static void trimNewline(char *text)
{
    size_t len;

    if (text == 0)
    {
        return;
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r'))
    {
        text[len - 1] = '\0';
        len--;
    }
}

/*
 * 函数名称：readText
 * 函数功能：读取一段非空文本输入。
 * 参数说明：prompt 为提示语；buffer 为保存输入的缓冲区；size 为缓冲区大小。
 * 返回值：无。
 * 实现说明：若用户直接回车则提示重新输入，避免发送空字段给服务器。
 */
static void readText(const char *prompt, char *buffer, int size)
{
    while (1)
    {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == 0)
        {
            buffer[0] = '\0';
            return;
        }
        trimNewline(buffer);
        if (buffer[0] != '\0')
        {
            return;
        }
        printf("输入不能为空，请重新输入。\n");
    }
}

/*
 * 函数名称：readIntValue
 * 函数功能：读取整数菜单项或数量。
 * 参数说明：prompt 为输入提示语。
 * 返回值：返回用户输入转换后的整数。
 * 实现说明：先按字符串读取，再用 atoi 转换，保持菜单输入处理简单统一。
 */
static int readIntValue(const char *prompt)
{
    char buffer[64];

    readText(prompt, buffer, sizeof(buffer));
    return atoi(buffer);
}

/*
 * 函数名称：printResponse
 * 函数功能：统一打印服务器返回的业务结果。
 * 参数说明：response 为服务器响应包。
 * 返回值：无。
 * 实现说明：所有响应内容都由服务器写入 data 字段，客户端只负责显示。
 */
static void printResponse(const Packet *response)
{
    printf("\n%s\n", response->data);
}

/*
 * 函数名称：sendSimple
 * 函数功能：封装无复杂返回处理的普通请求发送流程。
 * 参数说明：context 为客户端上下文；cmd 为命令码；data 为请求数据。
 * 返回值：业务成功返回 1，通信失败或业务失败返回 0。
 * 实现说明：适用于查询、删除、查看订单等“发一个包、显示结果”的功能。
 */
static int sendSimple(ClientContext *context, int cmd, const char *data)
{
    Packet request;
    Packet response;

    initPacket(&request, cmd);
    copyText(request.data, MAX_DATA, data);
    if (!requestServer(context, &request, &response))
    {
        return 0;
    }

    printResponse(&response);
    return response.result == RESULT_OK;
}

/*
 * 函数名称：doRegister
 * 函数功能：处理用户注册菜单输入并发送注册请求。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：将用户名和密码按“用户名|密码”的格式写入 Packet.data。
 */
static void doRegister(ClientContext *context)
{
    char username[50];
    char password[50];
    char data[MAX_DATA];

    readText("用户名：", username, sizeof(username));
    readText("密码：", password, sizeof(password));
    sprintf_s(data, sizeof(data), "%s|%s", username, password);
    (void)sendSimple(context, CMD_REGISTER, data);
}

/*
 * 函数名称：doLogin
 * 函数功能：处理普通用户或管理员登录。
 * 参数说明：context 为客户端上下文；adminMode 为 1 表示管理员登录入口。
 * 返回值：无。
 * 实现说明：登录成功后保存用户名、登录状态和管理员标记，供后续菜单判断权限。
 */
static void doLogin(ClientContext *context, int adminMode)
{
    Packet request;
    Packet response;
    char username[50];
    char password[50];

    readText("用户名：", username, sizeof(username));
    readText("密码：", password, sizeof(password));
    initPacket(&request, adminMode ? CMD_ADMIN_LOGIN : CMD_LOGIN);
    sprintf_s(request.data, sizeof(request.data), "%s|%s", username, password);

    if (requestServer(context, &request, &response))
    {
        printResponse(&response);
        if (response.result == RESULT_OK)
        {
            context->loginState = 1;
            context->isAdmin = strstr(response.data, "管理员") != 0;
            copyText(context->username, (int)sizeof(context->username), username);
        }
    }
}

/*
 * 函数名称：queryFlight
 * 函数功能：按关键字查询航班，关键字为空时查询全部航班。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：关键字可以是航班号、出发地或目的地，由服务器负责匹配。
 */
static void queryFlight(ClientContext *context)
{
    char keyword[80];

    printf("关键字可输入航班号、出发地、目的地，直接回车查询全部：");
    if (fgets(keyword, sizeof(keyword), stdin) == 0)
    {
        keyword[0] = '\0';
    }
    trimNewline(keyword);
    (void)sendSimple(context, CMD_QUERY_FLIGHT, keyword);
}

/*
 * 函数名称：addOrUpdateFlight
 * 函数功能：采集航班信息并发送新增或修改航班请求。
 * 参数说明：context 为客户端上下文；updateMode 为 1 表示修改，为 0 表示新增。
 * 返回值：无。
 * 实现说明：航班字段使用竖线分隔，便于服务器解析并写入数据文件。
 */
static void addOrUpdateFlight(ClientContext *context, int updateMode)
{
    char flightNo[20];
    char startCity[50];
    char endCity[50];
    char date[20];
    char startTime[20];
    char arriveTime[20];
    char status[20];
    int totalSeat;
    int remainSeat;
    float price;
    char data[MAX_DATA];

    readText("航班号：", flightNo, sizeof(flightNo));
    readText("出发地：", startCity, sizeof(startCity));
    readText("目的地：", endCity, sizeof(endCity));
    readText("日期(YYYY-MM-DD)：", date, sizeof(date));
    readText("起飞时间(HH:MM)：", startTime, sizeof(startTime));
    readText("到达时间(HH:MM)：", arriveTime, sizeof(arriveTime));
    totalSeat = readIntValue("总座位数：");
    remainSeat = readIntValue("余票数：");
    printf("票价：");
    scanf_s("%f", &price);
    getchar();
    readText("状态：", status, sizeof(status));

    sprintf_s(data, sizeof(data), "%s|%s|%s|%s|%s|%s|%d|%d|%.2f|%s",
        flightNo, startCity, endCity, date, startTime, arriveTime,
        totalSeat, remainSeat, price, status);
    (void)sendSimple(context, updateMode ? CMD_UPDATE_FLIGHT : CMD_ADD_FLIGHT, data);
}

/*
 * 函数名称：deleteFlight
 * 函数功能：根据航班号删除航班。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：客户端只提交航班号，是否存在和是否有权限由服务器判断。
 */
static void deleteFlight(ClientContext *context)
{
    char flightNo[20];

    readText("要删除的航班号：", flightNo, sizeof(flightNo));
    (void)sendSimple(context, CMD_DELETE_FLIGHT, flightNo);
}

/*
 * 函数名称：bookTicket
 * 函数功能：采集乘客和航班信息并发送订票请求。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：服务器会在临界区内检查余票，余票不足时自动加入候补队列。
 */
static void bookTicket(ClientContext *context)
{
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;
    char data[MAX_DATA];

    readText("乘客姓名：", name, sizeof(name));
    readText("手机号：", phone, sizeof(phone));
    readText("航班号：", flightNo, sizeof(flightNo));
    ticketNum = readIntValue("票数：");
    sprintf_s(data, sizeof(data), "%s|%s|%s|%d", name, phone, flightNo, ticketNum);
    (void)sendSimple(context, CMD_BOOK, data);
}

/*
 * 函数名称：cancelTicket
 * 函数功能：根据订单号发送退票请求。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：退票成功后服务器会恢复余票，并尝试为候补队列自动出票。
 */
static void cancelTicket(ClientContext *context)
{
    char orderId[30];

    readText("退票订单号：", orderId, sizeof(orderId));
    (void)sendSimple(context, CMD_CANCEL, orderId);
}

/*
 * 函数名称：userMenu
 * 函数功能：显示并处理普通用户功能菜单。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：循环读取菜单选择，通过 switch 分发到查询、订票、退票等功能。
 */
static void userMenu(ClientContext *context)
{
    int choice;

    while (1)
    {
        printf("\n======== 用户菜单 ========\n");
        printf("1. 查询航班\n");
        printf("2. 订票\n");
        printf("3. 退票\n");
        printf("4. 查看我的订单\n");
        printf("5. 查看候补队列\n");
        printf("0. 返回\n");
        choice = readIntValue("请选择：");

        switch (choice)
        {
        case 1:
            queryFlight(context);
            break;
        case 2:
            bookTicket(context);
            break;
        case 3:
            cancelTicket(context);
            break;
        case 4:
            (void)sendSimple(context, CMD_VIEW_ORDER, "");
            break;
        case 5:
            (void)sendSimple(context, CMD_WAITLIST, "");
            break;
        case 0:
            return;
        default:
            printf("无效选择。\n");
            break;
        }
    }
}

/*
 * 函数名称：adminMenu
 * 函数功能：显示并处理管理员功能菜单。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：管理员菜单只在管理员登录成功后进入，具体权限仍由服务器二次校验。
 */
static void adminMenu(ClientContext *context)
{
    int choice;

    while (1)
    {
        printf("\n======== 管理员菜单 ========\n");
        printf("1. 查看所有航班\n");
        printf("2. 新增航班\n");
        printf("3. 删除航班\n");
        printf("4. 修改航班\n");
        printf("5. 查看所有用户\n");
        printf("6. 查看所有订单\n");
        printf("7. 查看候补统计\n");
        printf("8. 查看销售统计\n");
        printf("0. 返回\n");
        choice = readIntValue("请选择：");

        switch (choice)
        {
        case 1:
            (void)sendSimple(context, CMD_QUERY_FLIGHT, "");
            break;
        case 2:
            addOrUpdateFlight(context, 0);
            break;
        case 3:
            deleteFlight(context);
            break;
        case 4:
            addOrUpdateFlight(context, 1);
            break;
        case 5:
            (void)sendSimple(context, CMD_VIEW_USERS, "");
            break;
        case 6:
            (void)sendSimple(context, CMD_VIEW_ALL_ORDERS, "");
            break;
        case 7:
            (void)sendSimple(context, CMD_WAITLIST, "");
            break;
        case 8:
            (void)sendSimple(context, CMD_STATISTICS, "");
            break;
        case 0:
            return;
        default:
            printf("无效选择。\n");
            break;
        }
    }
}

/*
 * 函数名称：runClientUi
 * 函数功能：客户端主菜单循环。
 * 参数说明：context 为客户端上下文。
 * 返回值：无。
 * 实现说明：负责注册、登录、管理员入口和未登录查询航班等顶层功能。
 */
void runClientUi(ClientContext *context)
{
    int choice;

    while (1)
    {
        printf("\n========================\n");
        printf("航空票务网络客户端\n");
        printf("========================\n");
        printf("1. 用户注册\n");
        printf("2. 用户登录\n");
        printf("3. 管理员登录\n");
        printf("4. 查询航班\n");
        printf("0. 退出\n");
        choice = readIntValue("请选择：");

        switch (choice)
        {
        case 1:
            doRegister(context);
            break;
        case 2:
            doLogin(context, 0);
            if (context->loginState)
            {
                userMenu(context);
            }
            break;
        case 3:
            doLogin(context, 1);
            if (context->loginState && context->isAdmin)
            {
                adminMenu(context);
            }
            break;
        case 4:
            queryFlight(context);
            break;
        case 0:
            return;
        default:
            printf("无效选择。\n");
            break;
        }
    }
}
