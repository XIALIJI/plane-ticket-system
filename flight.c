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
    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8s %-8s %-10s\n",
           "航班号", "出发地", "目的地", "日期", "起飞", "到达", "总票数", "余票", "票价");
    printf("------------------------------------------------------------------------------------------\n");
}

void printFlight(const Flight *flight)
{
    if (flight == NULL)
    {
        return;
    }

    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8d %-8d %-10.2f\n",
           flight->flightNo,
           flight->startCity,
           flight->endCity,
           flight->date,
           flight->startTime,
           flight->arriveTime,
           flight->totalSeat,
           flight->remainSeat,
           flight->price);
}

static void showFlightArray(Flight flights[], int count)
{
    int i;

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

void showAllFlights(void)
{
    Flight flights[MAX_FLIGHTS];
    int count = loadFlights(flights, MAX_FLIGHTS);
    showFlightArray(flights, count);
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
    flight.price = readFloat("请输入票价：");

    if (flight.totalSeat < 0 || flight.remainSeat < 0 || flight.remainSeat > flight.totalSeat || flight.price < 0)
    {
        printf("航班数据不合法，新增失败。\n");
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
    flight->price = readFloat("请输入新的票价：");

    if (flight->totalSeat < 0 || flight->remainSeat < 0 || flight->remainSeat > flight->totalSeat || flight->price < 0)
    {
        printf("航班数据不合法，修改失败。\n");
        return;
    }

    if (saveFlights(flights, count))
    {
        printf("修改航班成功。\n");
    }
}

static void bubbleSort(Flight flights[], int count, int sortType)
{
    int i;
    int j;

    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - 1 - i; j++)
        {
            int needSwap = 0;

            if (sortType == 1 && strcmp(flights[j].flightNo, flights[j + 1].flightNo) > 0)
            {
                needSwap = 1;
            }
            else if (sortType == 2 && flights[j].remainSeat > flights[j + 1].remainSeat)
            {
                needSwap = 1;
            }
            else if (sortType == 3 && flights[j].price > flights[j + 1].price)
            {
                needSwap = 1;
            }
            else if (sortType == 4 && strcmp(flights[j].startTime, flights[j + 1].startTime) > 0)
            {
                needSwap = 1;
            }

            if (needSwap)
            {
                Flight temp = flights[j];
                flights[j] = flights[j + 1];
                flights[j + 1] = temp;
            }
        }
    }
}

static void sortAndShow(int sortType)
{
    Flight flights[MAX_FLIGHTS];
    int count = loadFlights(flights, MAX_FLIGHTS);

    bubbleSort(flights, count, sortType);
    showFlightArray(flights, count);
}

void sortFlightByNo(void)
{
    sortAndShow(1);
}

void sortFlightByRemainSeat(void)
{
    sortAndShow(2);
}

void sortFlightByPrice(void)
{
    sortAndShow(3);
}

void sortFlightByStartTime(void)
{
    sortAndShow(4);
}

void sortFlightMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n========================\n");
        printf("航班排序\n");
        printf("========================\n");
        printf("1.按航班号排序\n");
        printf("2.按余票排序\n");
        printf("3.按票价排序\n");
        printf("4.按起飞时间排序\n");
        printf("0.返回\n");

        choice = readInt("请选择：");
        switch (choice)
        {
        case 1:
            sortFlightByNo();
            break;
        case 2:
            sortFlightByRemainSeat();
            break;
        case 3:
            sortFlightByPrice();
            break;
        case 4:
            sortFlightByStartTime();
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重新输入。\n");
            break;
        }
    }
}
