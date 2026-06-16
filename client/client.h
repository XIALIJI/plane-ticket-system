#ifndef PLANE_CLIENT_H
#define PLANE_CLIENT_H

#include "../common/packet.h"

#include <winsock2.h>

typedef struct
{
    SOCKET sock;
    char username[50];
    int loginState;
    int isAdmin;
} ClientContext;

int connectServer(ClientContext *context);
void closeClient(ClientContext *context);
int requestServer(ClientContext *context, Packet *request, Packet *response);

#endif
