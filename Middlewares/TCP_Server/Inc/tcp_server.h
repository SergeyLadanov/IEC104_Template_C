#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include <stdint.h>
#include <winsock.h>

typedef struct __TCP_Server
{
    SOCKET Server_Fd, Client_fd;
    struct sockaddr_in Server, Client;
    uint16_t ConNum;
    uint16_t MaxConnections;
} TCP_Server;

bool TCP_IsConnected(SOCKET* sock);


#endif