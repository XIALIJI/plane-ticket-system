#include "queue.h"
#include "file.h"
#include "flight.h"
#include "passenger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static WaitNode *front = NULL;
static WaitNode *rear = NULL;

static WaitNode *createNode(const char *name, const char *phone, const char *flightNo, int ticketNum)
{
    WaitNode *node = (WaitNode *)malloc(sizeof(WaitNode));
    if (node == NULL)
    {
        printf("错误：内存分配失败。\n");
        return NULL;
    }

    safeCopy(node->name, sizeof(node->name), name);
    safeCopy(node->phone, sizeof(node->phone), phone);
    safeCopy(node->flightNo, sizeof(node->flightNo), flightNo);
    node->ticketNum = ticketNum;
    node->next = NULL;
    return node;
}

static void appendNodeToMemory(WaitNode *node)
{
    if (node == NULL)
    {
        return;
    }

    if (rear == NULL)
    {
        front = node;
        rear = node;
    }
    else
    {
        rear->next = node;
        rear = node;
    }
}

void clearQueue(void)
{
    WaitNode *current = front;
    while (current != NULL)
    {
        WaitNode *next = current->next;
        free(current);
        current = next;
    }
    front = NULL;
    rear = NULL;
}

static int parseWaitLine(const char *line, WaitNode *node)
{
    if (line == NULL || node == NULL)
    {
        return 0;
    }

#ifdef _MSC_VER
    return sscanf_s(line, "%99s %29s %19s %d",
                    node->name, (unsigned)_countof(node->name),
                    node->phone, (unsigned)_countof(node->phone),
                    node->flightNo, (unsigned)_countof(node->flightNo),
                    &node->ticketNum) == 4;
#else
    return sscanf(line, "%99s %29s %19s %d",
                  node->name, node->phone, node->flightNo, &node->ticketNum) == 4;
#endif
}

static int saveWaitQueue(void)
{
    FILE *fp;
    WaitNode *current;

    fp = fopen(WAITLIST_FILE, "w");
    if (fp == NULL)
    {
        printf("错误：无法打开 %s 写入。\n", WAITLIST_FILE);
        return 0;
    }

    current = front;
    while (current != NULL)
    {
        fprintf(fp, "%s %s %s %d\n",
                current->name,
                current->phone,
                current->flightNo,
                current->ticketNum);
        current = current->next;
    }

    fclose(fp);
    return 1;
}

void initQueue(void)
{
    FILE *fp;
    char line[512];

    clearQueue();
    fp = fopen(WAITLIST_FILE, "r");
    if (fp == NULL)
    {
        printf("提示：未找到 %s，将按空候补队列处理。\n", WAITLIST_FILE);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        WaitNode temp;
        WaitNode *node;
        if (!parseWaitLine(line, &temp))
        {
            continue;
        }

        node = createNode(temp.name, temp.phone, temp.flightNo, temp.ticketNum);
        if (node == NULL)
        {
            fclose(fp);
            return;
        }
        appendNodeToMemory(node);
    }

    fclose(fp);
}

int isQueueEmpty(void)
{
    return front == NULL;
}

int enqueueWait(const char *name, const char *phone, const char *flightNo, int ticketNum)
{
    WaitNode *node;

    if (ticketNum <= 0)
    {
        printf("候补票数必须大于 0。\n");
        return 0;
    }

    initQueue();
    node = createNode(name, phone, flightNo, ticketNum);
    if (node == NULL)
    {
        return 0;
    }

    appendNodeToMemory(node);
    if (!saveWaitQueue())
    {
        return 0;
    }

    return 1;
}

void enqueue(void)
{
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;

    readString("请输入姓名：", name, sizeof(name));
    readString("请输入手机号：", phone, sizeof(phone));
    readString("请输入航班号：", flightNo, sizeof(flightNo));
    ticketNum = readInt("请输入候补票数：");

    if (enqueueWait(name, phone, flightNo, ticketNum))
    {
        printf("加入候补成功。\n");
    }
}

void dequeue(void)
{
    WaitNode *node;

    initQueue();
    if (isQueueEmpty())
    {
        printf("候补队列为空。\n");
        return;
    }

    node = front;
    front = front->next;
    if (front == NULL)
    {
        rear = NULL;
    }

    printf("已移除候补：%s %s %s %d\n", node->name, node->phone, node->flightNo, node->ticketNum);
    free(node);
    (void)saveWaitQueue();
}

void showWaitQueue(void)
{
    WaitNode *current;

    initQueue();
    if (isQueueEmpty())
    {
        printf("候补名单为空。\n");
        return;
    }

    printf("%-16s %-16s %-12s %-8s\n", "姓名", "手机号", "航班号", "票数");
    printf("--------------------------------------------------------\n");
    current = front;
    while (current != NULL)
    {
        printf("%-16s %-16s %-12s %-8d\n",
               current->name,
               current->phone,
               current->flightNo,
               current->ticketNum);
        current = current->next;
    }
}

void autoBookFromQueue(const char *flightNo)
{
    Flight flights[MAX_FLIGHTS];
    int flightCount;
    int flightIndex;
    WaitNode *current;
    WaitNode *previous = NULL;
    int bookedCount = 0;

    if (flightNo == NULL)
    {
        return;
    }

    initQueue();
    if (isQueueEmpty())
    {
        return;
    }

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    flightIndex = findFlightIndexByNo(flights, flightCount, flightNo);
    if (flightIndex < 0)
    {
        return;
    }

    current = front;
    while (current != NULL)
    {
        if (strcmp(current->flightNo, flightNo) == 0 &&
            flights[flightIndex].remainSeat >= current->ticketNum)
        {
            Passenger passenger;
            WaitNode *bookedNode = current;

            safeCopy(passenger.name, sizeof(passenger.name), current->name);
            safeCopy(passenger.phone, sizeof(passenger.phone), current->phone);
            safeCopy(passenger.flightNo, sizeof(passenger.flightNo), current->flightNo);
            passenger.ticketNum = current->ticketNum;

            if (!appendPassenger(&passenger))
            {
                previous = current;
                current = current->next;
                continue;
            }

            flights[flightIndex].remainSeat -= current->ticketNum;
            bookedCount++;
            printf("候补自动出票成功：%s %s %s %d 张。\n",
                   passenger.name, passenger.phone, passenger.flightNo, passenger.ticketNum);

            current = current->next;
            if (previous == NULL)
            {
                front = current;
            }
            else
            {
                previous->next = current;
            }

            if (bookedNode == rear)
            {
                rear = previous;
            }
            free(bookedNode);
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }

    if (bookedCount > 0)
    {
        (void)saveFlights(flights, flightCount);
        (void)saveWaitQueue();
    }
}
