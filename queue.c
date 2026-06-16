#include "queue.h"
#include "file.h"
#include "flight.h"
#include "passenger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static WaitNode *front = NULL;
static WaitNode *rear = NULL;

/*
 * 函数名称：createNode
 * 函数功能：创建一个候补队列节点。
 * 参数说明：name 为姓名；phone 为手机号；flightNo 为航班号；ticketNum 为候补票数。
 * 返回值：创建成功返回节点指针，失败返回 NULL。
 * 实现说明：动态分配链表节点，并用安全复制函数写入各字段。
 */
static WaitNode *createNode(const char *name, const char *phone, const char *flightNo, int ticketNum)
{
    WaitNode *node = (WaitNode *)malloc(sizeof(WaitNode));
    if (node == NULL)
    {
        printf("閿欒锛氬唴瀛樺垎閰嶅け璐ャ€俓n");
        return NULL;
    }

    safeCopy(node->name, sizeof(node->name), name);
    safeCopy(node->phone, sizeof(node->phone), phone);
    safeCopy(node->flightNo, sizeof(node->flightNo), flightNo);
    node->ticketNum = ticketNum;
    node->next = NULL;
    return node;
}

/*
 * 函数名称：appendNodeToMemory
 * 函数功能：将节点追加到内存候补队列尾部。
 * 参数说明：node 为待追加节点。
 * 返回值：无。
 * 实现说明：队列为空时 front 和 rear 同时指向新节点，否则追加到 rear 后面。
 */
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

/*
 * 函数名称：clearQueue
 * 函数功能：释放内存中的候补队列。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：逐个释放链表节点，最后将 front 和 rear 置空。
 */
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

/*
 * 函数名称：parseWaitLine
 * 函数功能：解析候补文件中的一行记录。
 * 参数说明：line 为文件行文本；node 为解析后的临时节点。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：候补文件按姓名、电话、航班号、票数四个字段保存。
 */
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

/*
 * 函数名称：saveWaitQueue
 * 函数功能：将内存候补队列保存到文件。
 * 参数说明：无。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：遍历链表，把每个候补节点按一行写入 waitlist.txt。
 */
static int saveWaitQueue(void)
{
    FILE *fp;
    WaitNode *current;

    fp = fopen(WAITLIST_FILE, "w");
    if (fp == NULL)
    {
        printf("閿欒锛氭棤娉曟墦寮€ %s 鍐欏叆銆俓n", WAITLIST_FILE);
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

/*
 * 函数名称：initQueue
 * 函数功能：从候补文件初始化内存队列。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：先清空旧链表，再读取文件逐条创建节点并追加到队尾。
 */
void initQueue(void)
{
    FILE *fp;
    char line[512];

    clearQueue();
    fp = fopen(WAITLIST_FILE, "r");
    if (fp == NULL)
    {
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

/*
 * 函数名称：isQueueEmpty
 * 函数功能：判断候补队列是否为空。
 * 参数说明：无。
 * 返回值：为空返回 1，否则返回 0。
 * 实现说明：只需判断 front 指针是否为 NULL。
 */
int isQueueEmpty(void)
{
    return front == NULL;
}

/*
 * 函数名称：getWaitQueueCount
 * 函数功能：统计候补队列记录数量。
 * 参数说明：无。
 * 返回值：返回候补节点数量。
 * 实现说明：先从文件刷新队列，再遍历链表计数。
 */
int getWaitQueueCount(void)
{
    WaitNode *current;
    int count = 0;

    initQueue();
    current = front;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    return count;
}

/*
 * 函数名称：enqueueWait
 * 函数功能：把指定乘客加入候补队列。
 * 参数说明：name 为姓名；phone 为手机号；flightNo 为航班号；ticketNum 为票数。
 * 返回值：加入成功返回 1，失败返回 0。
 * 实现说明：先刷新队列，再创建节点追加到队尾并保存文件。
 */
int enqueueWait(const char *name, const char *phone, const char *flightNo, int ticketNum)
{
    WaitNode *node;

    if (ticketNum <= 0)
    {
        printf("鍊欒ˉ绁ㄦ暟蹇呴』澶т簬 0銆俓n");
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

/*
 * 函数名称：enqueue
 * 函数功能：通过控制台输入手动加入候补。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：读取姓名、电话、航班号和票数后调用 enqueueWait 完成入队。
 */
void enqueue(void)
{
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;

    readString("璇疯緭鍏ュ鍚嶏細", name, sizeof(name));
    readString("璇疯緭鍏ユ墜鏈哄彿锛?, phone, sizeof(phone));
    readString("璇疯緭鍏ヨ埅鐝彿锛?, flightNo, sizeof(flightNo));
    ticketNum = readInt("璇疯緭鍏ュ€欒ˉ绁ㄦ暟锛?);

    if (enqueueWait(name, phone, flightNo, ticketNum))
    {
        printf("鍔犲叆鍊欒ˉ鎴愬姛銆俓n");
    }
}

/*
 * 函数名称：dequeue
 * 函数功能：移除候补队列队首记录。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：队首出队后释放节点，并重新保存候补文件。
 */
void dequeue(void)
{
    WaitNode *node;

    initQueue();
    if (isQueueEmpty())
    {
        printf("鍊欒ˉ闃熷垪涓虹┖銆俓n");
        return;
    }

    node = front;
    front = front->next;
    if (front == NULL)
    {
        rear = NULL;
    }

    printf("宸茬Щ闄ゅ€欒ˉ锛?s %s %s %d\n", node->name, node->phone, node->flightNo, node->ticketNum);
    free(node);
    (void)saveWaitQueue();
}

/*
 * 函数名称：showWaitQueue
 * 函数功能：显示候补队列内容。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：先从文件刷新队列，再按链表顺序逐条输出候补信息。
 */
void showWaitQueue(void)
{
    WaitNode *current;

    initQueue();
    if (isQueueEmpty())
    {
        printf("鍊欒ˉ鍚嶅崟涓虹┖銆俓n");
        return;
    }

    printf("%-16s %-16s %-12s %-8s\n", "濮撳悕", "鎵嬫満鍙?, "鑸彮鍙?, "绁ㄦ暟");
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

/*
 * 函数名称：autoBookFromQueue
 * 函数功能：退票后自动为候补队列出票。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：从队首开始检查余票，余票足够就生成乘客订单、扣减余票并移除候补节点。
 */
void autoBookFromQueue(void)
{
    Flight flights[MAX_FLIGHTS];
    int flightCount;
    int bookedCount = 0;

    initQueue();
    if (isQueueEmpty())
    {
        return;
    }

    flightCount = loadFlights(flights, MAX_FLIGHTS);

    while (front != NULL)
    {
        int flightIndex = findFlightIndexByNo(flights, flightCount, front->flightNo);

        if (flightIndex < 0 || flights[flightIndex].remainSeat < front->ticketNum)
        {
            break;
        }

        {
            Passenger passenger;
            WaitNode *bookedNode = front;

            generateOrderId(passenger.orderId, sizeof(passenger.orderId));
            safeCopy(passenger.name, sizeof(passenger.name), front->name);
            safeCopy(passenger.phone, sizeof(passenger.phone), front->phone);
            safeCopy(passenger.flightNo, sizeof(passenger.flightNo), front->flightNo);
            passenger.ticketNum = front->ticketNum;

            if (!appendPassenger(&passenger))
            {
                break;
            }

            flights[flightIndex].remainSeat -= front->ticketNum;
            front = front->next;
            if (front == NULL)
            {
                rear = NULL;
            }

            printf("鍊欒ˉ鑷姩鍑虹エ鎴愬姛锛氳鍗曞彿 %s锛?s %s %d 寮犮€俓n",
                   passenger.orderId, passenger.name, passenger.flightNo, passenger.ticketNum);

            free(bookedNode);
            bookedCount++;
        }
    }

    if (bookedCount > 0)
    {
        (void)saveFlights(flights, flightCount);
        (void)saveWaitQueue();
    }
}
