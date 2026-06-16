#ifndef PLANE_PROTOCOL_H
#define PLANE_PROTOCOL_H

#include "packet.h"

#ifdef _WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#endif

int sendPacket(SOCKET sock, const Packet *packet);
int recvPacket(SOCKET sock, Packet *packet);
void copyText(char *dest, int destSize, const char *src);
void appendText(char *dest, int destSize, const char *src);

#endif
