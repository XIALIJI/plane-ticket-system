#include "passenger.h"
#include "file.h"
#include "flight.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void printPassengerHeader(void)
{
    printf("%-18s %-16s %-16s %-12s %-8s\n", "订单号", "姓名", "手机号", "航班号", "票数");
    printf("--------------------------------------------------------------------------\n");
}

static void printPassenger(const Passenger *passenger)
{
    if (passenger == NULL)
    {
        return;
    }

    printf("%-18s %-16s %-16s %-12s %-8d\n",
           passenger->orderId,
           passenger->name,
           passenger->phone,
           passenger->flightNo,
           passenger->ticketNum);
}

static void getTodayText(char *dateText, int dateTextSize)
{
    time_t now;
    struct tm localTime;

    if (dateText == NULL || dateTextSize <= 0)
    {
        return;
    }

    now = time(NULL);
#ifdef _MSC_VER
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif

    snprintf(dateText, (size_t)dateTextSize, "%04d%02d%02d",
             localTime.tm_year + 1900,
             localTime.tm_mon + 1,
             localTime.tm_mday);
}

void generateOrderId(char *orderId, int orderIdSize)
{
    Passenger passengers[MAX_PASSENGERS];
    char dateText[16];
    char prefix[20];
    int count;
    int i;
    int maxSerial = 0;

    if (orderId == NULL || orderIdSize <= 0)
    {
        return;
    }

    getTodayText(dateText, sizeof(dateText));
    snprintf(prefix, sizeof(prefix), "OD%s", dateText);

    count = loadPassengers(passengers, MAX_PASSENGERS);
    for (i = 0; i < count; i++)
    {
        if (strncmp(passengers[i].orderId, prefix, strlen(prefix)) == 0)
        {
            int serial = atoi(passengers[i].orderId + strlen(prefix));
            if (serial > maxSerial)
            {
                maxSerial = serial;
            }
        }
    }

    snprintf(orderId, (size_t)orderIdSize, "%s%04d", prefix, maxSerial + 1);
}

void showPassengers(void)
{
    Passenger passengers[MAX_PASSENGERS];
    int count;
    int i;

    count = loadPassengers(passengers, MAX_PASSENGERS);
    if (count == 0)
    {
        printf("暂无乘客信息。\n");
        return;
    }

    printPassengerHeader();
    for (i = 0; i < count; i++)
    {
        printPassenger(&passengers[i]);
    }
}

void bookTicket(void)
{
    Flight flights[MAX_FLIGHTS];
    Passenger passenger;
    int flightCount;
    int index;
    float totalAmount;

    generateOrderId(passenger.orderId, sizeof(passenger.orderId));
    readString("请输入姓名：", passenger.name, sizeof(passenger.name));
    readString("请输入手机号：", passenger.phone, sizeof(passenger.phone));
    readString("请输入航班号：", passenger.flightNo, sizeof(passenger.flightNo));
    passenger.ticketNum = readInt("请输入票数：");

    if (passenger.ticketNum <= 0)
    {
        printf("票数必须大于 0。\n");
        return;
    }

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, flightCount, passenger.flightNo);
    if (index < 0)
    {
        printf("航班不存在，订票失败。\n");
        return;
    }

    if (flights[index].remainSeat < passenger.ticketNum)
    {
        printf("余票不足，当前余票 %d 张。\n", flights[index].remainSeat);
        if (enqueueWait(passenger.name, passenger.phone, passenger.flightNo, passenger.ticketNum))
        {
            printf("已自动加入候补名单。\n");
        }
        return;
    }

    flights[index].remainSeat -= passenger.ticketNum;
    if (!saveFlights(flights, flightCount))
    {
        printf("保存航班信息失败，订票取消。\n");
        return;
    }

    if (!appendPassenger(&passenger))
    {
        flights[index].remainSeat += passenger.ticketNum;
        (void)saveFlights(flights, flightCount);
        printf("保存乘客订单失败，订票取消。\n");
        return;
    }

    totalAmount = flights[index].price * passenger.ticketNum;
    printf("订票成功。\n");
    printf("订单号：%s\n", passenger.orderId);
    printf("票价：%.2f\n", flights[index].price);
    printf("订票数量：%d\n", passenger.ticketNum);
    printf("总金额：%.2f元\n", totalAmount);
}

void refundTicket(void)
{
    Passenger passengers[MAX_PASSENGERS];
    Flight flights[MAX_FLIGHTS];
    char orderId[30];
    char refundedFlightNo[20];
    int passengerCount;
    int flightCount;
    int passengerIndex = -1;
    int flightIndex;
    int i;
    int refundNum;

    readString("请输入退票订单号：", orderId, sizeof(orderId));
    passengerCount = loadPassengers(passengers, MAX_PASSENGERS);

    for (i = 0; i < passengerCount; i++)
    {
        if (strcmp(passengers[i].orderId, orderId) == 0)
        {
            passengerIndex = i;
            break;
        }
    }

    if (passengerIndex < 0)
    {
        printf("未找到该订单号对应的订单。\n");
        return;
    }

    safeCopy(refundedFlightNo, sizeof(refundedFlightNo), passengers[passengerIndex].flightNo);
    refundNum = passengers[passengerIndex].ticketNum;

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    flightIndex = findFlightIndexByNo(flights, flightCount, refundedFlightNo);
    if (flightIndex >= 0)
    {
        flights[flightIndex].remainSeat += refundNum;
        if (flights[flightIndex].remainSeat > flights[flightIndex].totalSeat)
        {
            flights[flightIndex].remainSeat = flights[flightIndex].totalSeat;
        }
        (void)saveFlights(flights, flightCount);
    }

    for (i = passengerIndex; i < passengerCount - 1; i++)
    {
        passengers[i] = passengers[i + 1];
    }
    passengerCount--;

    if (!savePassengers(passengers, passengerCount))
    {
        printf("保存乘客信息失败，退票未完成。\n");
        return;
    }

    printf("退票成功，已恢复余票 %d 张。\n", refundNum);
    autoBookFromQueue();
}
