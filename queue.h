#ifndef QUEUE_H
#define QUEUE_H

typedef struct WaitNode
{
    char name[100];
    char phone[30];
    char flightNo[20];

    int ticketNum;

    struct WaitNode *next;

} WaitNode;

void initQueue(void);
int isQueueEmpty(void);
void enqueue(void);
void dequeue(void);
void showWaitQueue(void);
void autoBookFromQueue(void);
void clearQueue(void);

int enqueueWait(const char *name, const char *phone, const char *flightNo, int ticketNum);
int getWaitQueueCount(void);

#endif
