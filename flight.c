#include "flight.h"
#include "file.h"

#include <stdio.h>
#include <string.h>

int findFlightIndexByNo(Flight flights[], int count, const char *flightNo)
{
    int i;

    if (flights == NULL || flightNo == NULL)
    {
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(flights[i].flightNo, flightNo) == 0)
        {
            return i;
        }
    }

    return -1;
}

void printFlightHeader(void)
{
    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8s %-8s\n",
           "航班号", "出发地", "目的地", "日期", "起飞", "到达", "总票数", "余票");
    printf("--------------------------------------------------------------------------------\n");
}

void printFlight(const Flight *flight)
{
    if (flight == NULL)
    {
        return;
    }

    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8d %-8d\n",
           flight->flightNo,
           flight->startCity,
           flight->endCity,
           flight->date,
           flight->startTime,
           flight->arriveTime,
           flight->totalSeat,
           flight->remainSeat);
}

void showAllFlights(void)
{
    Flight flights[MAX_FLIGHTS];
    int count;
    int i;

    count = loadFlights(flights, MAX_FLIGHTS);
    if (count == 0)
    {
        printf("暂无航班信息。\n");
        return;
    }

    printFlightHeader();
    for (i = 0; i < count; i++)
    {
        printFlight(&flights[i]);
    }
}

void searchFlightByNo(void)
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;

    readString("请输入航班号：", flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("未找到航班号为 %s 的航班。\n", flightNo);
        return;
    }

    printFlightHeader();
    printFlight(&flights[index]);
}

void searchFlightByDestination(void)
{
    Flight flights[MAX_FLIGHTS];
    char destination[50];
    int count;
    int i;
    int found = 0;

    readString("请输入目的地：", destination, sizeof(destination));
    count = loadFlights(flights, MAX_FLIGHTS);

    for (i = 0; i < count; i++)
    {
        if (strcmp(flights[i].endCity, destination) == 0)
        {
            if (!found)
            {
                printFlightHeader();
            }
            printFlight(&flights[i]);
            found = 1;
        }
    }

    if (!found)
    {
        printf("未找到目的地为 %s 的航班。\n", destination);
    }
}

void addFlight(void)
{
    Flight flights[MAX_FLIGHTS];
    Flight flight;
    int count;

    count = loadFlights(flights, MAX_FLIGHTS);
    if (count >= MAX_FLIGHTS)
    {
        printf("航班数量已达上限，无法新增。\n");
        return;
    }

    readString("请输入航班号：", flight.flightNo, sizeof(flight.flightNo));
    if (findFlightIndexByNo(flights, count, flight.flightNo) >= 0)
    {
        printf("航班号已存在，新增失败。\n");
        return;
    }

    readString("请输入出发地：", flight.startCity, sizeof(flight.startCity));
    readString("请输入目的地：", flight.endCity, sizeof(flight.endCity));
    readString("请输入日期(YYYY-MM-DD)：", flight.date, sizeof(flight.date));
    readString("请输入起飞时间(HH:MM)：", flight.startTime, sizeof(flight.startTime));
    readString("请输入到达时间(HH:MM)：", flight.arriveTime, sizeof(flight.arriveTime));
    flight.totalSeat = readInt("请输入总座位数：");
    flight.remainSeat = readInt("请输入余票数：");

    if (flight.totalSeat < 0 || flight.remainSeat < 0 || flight.remainSeat > flight.totalSeat)
    {
        printf("座位数据不合法，新增失败。\n");
        return;
    }

    flights[count++] = flight;
    if (saveFlights(flights, count))
    {
        printf("新增航班成功。\n");
    }
}

void deleteFlight(void)
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;
    int i;

    readString("请输入要删除的航班号：", flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("航班不存在，删除失败。\n");
        return;
    }

    for (i = index; i < count - 1; i++)
    {
        flights[i] = flights[i + 1];
    }
    count--;

    if (saveFlights(flights, count))
    {
        printf("删除航班成功。\n");
    }
}

void modifyFlight(void)
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;
    Flight *flight;

    readString("请输入要修改的航班号：", flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("航班不存在，修改失败。\n");
        return;
    }

    flight = &flights[index];
    printf("当前航班信息：\n");
    printFlightHeader();
    printFlight(flight);

    readString("请输入新的出发地：", flight->startCity, sizeof(flight->startCity));
    readString("请输入新的目的地：", flight->endCity, sizeof(flight->endCity));
    readString("请输入新的日期(YYYY-MM-DD)：", flight->date, sizeof(flight->date));
    readString("请输入新的起飞时间(HH:MM)：", flight->startTime, sizeof(flight->startTime));
    readString("请输入新的到达时间(HH:MM)：", flight->arriveTime, sizeof(flight->arriveTime));
    flight->totalSeat = readInt("请输入新的总座位数：");
    flight->remainSeat = readInt("请输入新的余票数：");

    if (flight->totalSeat < 0 || flight->remainSeat < 0 || flight->remainSeat > flight->totalSeat)
    {
        printf("座位数据不合法，修改失败。\n");
        return;
    }

    if (saveFlights(flights, count))
    {
        printf("修改航班成功。\n");
    }
}
