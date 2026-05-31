#include "admin.h"
#include "file.h"
#include "flight.h"
#include "passenger.h"
#include "queue.h"

#include <conio.h>
#include <stdio.h>
#include <string.h>

static void readPassword(const char *prompt, char *password, int passwordSize)
{
    int index = 0;
    int ch;

    if (password == NULL || passwordSize <= 0)
    {
        return;
    }

    printf("%s", prompt);
    while (1)
    {
        ch = _getch();
        if (ch == '\r' || ch == '\n')
        {
            password[index] = '\0';
            printf("\n");
            return;
        }
        else if (ch == '\b')
        {
            if (index > 0)
            {
                index--;
                printf("\b \b");
            }
        }
        else if (index < passwordSize - 1)
        {
            password[index++] = (char)ch;
            printf("*");
        }
    }
}

int adminLogin(void)
{
    char username[50];
    char password[50];

    readString("请输入管理员账号：", username, sizeof(username));
    readPassword("请输入管理员密码：", password, (int)sizeof(password));

    if (strcmp(username, "admin") == 0 && strcmp(password, "123456") == 0)
    {
        printf("管理员登录成功。\n");
        return 1;
    }

    printf("账号或密码错误。\n");
    return 0;
}

void showStatistics(void)
{
    Flight flights[MAX_FLIGHTS];
    Passenger passengers[MAX_PASSENGERS];
    int flightCount;
    int passengerCount;
    int waitCount;
    int totalSeats = 0;
    int soldSeats = 0;
    int totalTickets = 0;
    float totalSales = 0.0f;
    float avgLoadRate = 0.0f;
    int i;

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    passengerCount = loadPassengers(passengers, MAX_PASSENGERS);
    waitCount = getWaitQueueCount();

    for (i = 0; i < flightCount; i++)
    {
        totalSeats += flights[i].totalSeat;
        soldSeats += flights[i].totalSeat - flights[i].remainSeat;
    }

    for (i = 0; i < passengerCount; i++)
    {
        int flightIndex = findFlightIndexByNo(flights, flightCount, passengers[i].flightNo);
        totalTickets += passengers[i].ticketNum;
        if (flightIndex >= 0)
        {
            totalSales += flights[flightIndex].price * passengers[i].ticketNum;
        }
    }

    if (totalSeats > 0)
    {
        avgLoadRate = (float)soldSeats * 100.0f / (float)totalSeats;
    }

    printf("\n========================\n");
    printf("统计信息\n");
    printf("========================\n");
    printf("总航班数：%d\n", flightCount);
    printf("总乘客数：%d\n", passengerCount);
    printf("总出票数：%d\n", totalTickets);
    printf("总候补人数：%d\n", waitCount);
    printf("总销售额：%.2f元\n", totalSales);
    printf("平均上座率：%.1f%%\n", avgLoadRate);
}

void adminMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n========================\n");
        printf("管理员功能\n");
        printf("========================\n");
        printf("1.新增航班\n");
        printf("2.删除航班\n");
        printf("3.修改航班\n");
        printf("4.统计信息\n");
        printf("5.航班排序\n");
        printf("0.返回\n");

        choice = readInt("请选择：");
        switch (choice)
        {
        case 1:
            addFlight();
            break;
        case 2:
            deleteFlight();
            break;
        case 3:
            modifyFlight();
            break;
        case 4:
            showStatistics();
            break;
        case 5:
            sortFlightMenu();
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重新输入。\n");
            break;
        }
    }
}
