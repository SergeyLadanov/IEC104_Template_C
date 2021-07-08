#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

typedef struct __TCP_Server
{
    SOCKET Server_Fd, Client_fd;
    struct sockaddr_in Server, Client;
    uint16_t ConNum;
    uint16_t MaxConnections;
    pthread_t Task;
    bool Active;
} TCP_Server;


typedef struct __TCP_Client
{
    SOCKET Client_Fd;
    struct sockaddr_in Client;
    pthread_mutex_t Mutex;
    pthread_t ReceiveTask;
    pthread_t PollTask;
    bool KeepLooping;
    void *Arg;
} TCP_Client;

int TCP_Init(TCP_Server *hs, uint32_t inadr, uint16_t port);
int TCP_Start(TCP_Server *hs);

int TCP_Client_Send(TCP_Client* hcl, uint8_t *buf, int len);

void TCP_OnReceived(TCP_Client *hcl, uint8_t *buf, uint16_t length);
void TCP_OnConnected(TCP_Client *hcl);
void TCP_OnClosed(TCP_Client *hcl);
void TCP_PollCon(TCP_Client *hcl);

#endif