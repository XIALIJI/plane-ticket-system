#ifndef PLANE_SERVER_H
#define PLANE_SERVER_H

#include "../common/packet.h"

#include <winsock2.h>
#include <windows.h>

#define MAX_USERS_COUNT 1000
#define MAX_FLIGHTS_COUNT 1000
#define MAX_ORDERS_COUNT 3000
#define MAX_WAIT_COUNT 3000

typedef struct
{
    char username[50];
    char password[50];
    int points;
    char level[20];
    int isAdmin;
} UserRecord;

typedef struct
{
    char flightNo[20];
    char startCity[50];
    char endCity[50];
    char date[20];
    char startTime[20];
    char arriveTime[20];
    int totalSeat;
    int remainSeat;
    float price;
    char status[20];
} FlightRecord;

typedef struct
{
    char orderId[30];
    char username[50];
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;
    float amount;
    char eticketNo[40];
    char status[20];
} OrderRecord;

typedef struct
{
    char username[50];
    char name[100];
    char phone[30];
    char flightNo[20];
    int ticketNum;
    int priority;
} WaitRecord;

typedef struct
{
    SOCKET sock;
    char username[50];
    int loginState;
    int isAdmin;
} Session;

void serverInitLocks(void);
void serverDeleteLocks(void);
DWORD WINAPI ClientThread(LPVOID lpParam);
void processPacket(Session *session, const Packet *request, Packet *response);

int handleRegister(const Packet *request, Packet *response);
int handleLogin(Session *session, const Packet *request, Packet *response);
int handleQueryFlight(const Packet *request, Packet *response);
int handleAddFlight(Session *session, const Packet *request, Packet *response);
int handleDeleteFlight(Session *session, const Packet *request, Packet *response);
int handleUpdateFlight(Session *session, const Packet *request, Packet *response);
int handleBook(Session *session, const Packet *request, Packet *response);
int handleCancel(Session *session, const Packet *request, Packet *response);
int handleOrder(Session *session, const Packet *request, Packet *response);
int handleWaitlist(Session *session, const Packet *request, Packet *response);
int handleViewUsers(Session *session, Packet *response);
int handleViewAllOrders(Session *session, Packet *response);
int handleStatistics(Session *session, Packet *response);

#endif
