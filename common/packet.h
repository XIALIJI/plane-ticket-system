#ifndef PLANE_PACKET_H
#define PLANE_PACKET_H

#include "config.h"

typedef enum
{
    CMD_REGISTER = 1,
    CMD_LOGIN,
    CMD_ADMIN_LOGIN,
    CMD_QUERY_FLIGHT,
    CMD_ADD_FLIGHT,
    CMD_DELETE_FLIGHT,
    CMD_UPDATE_FLIGHT,
    CMD_BOOK,
    CMD_CANCEL,
    CMD_VIEW_ORDER,
    CMD_WAITLIST,
    CMD_VIEW_USERS,
    CMD_VIEW_ALL_ORDERS,
    CMD_STATISTICS,
    CMD_EXIT
} CommandCode;

typedef enum
{
    RESULT_OK = 0,
    RESULT_ERROR = 1,
    RESULT_LOGIN_REQUIRED = 2,
    RESULT_ADMIN_REQUIRED = 3
} ResultCode;

typedef struct
{
    int cmd;
    int result;
    char username[50];
    char data[MAX_DATA];
} Packet;

void initPacket(Packet *packet, int cmd);

#endif
