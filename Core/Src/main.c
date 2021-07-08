#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdbool.h>
#include "iec104.h"
#include "iec104_model.h"
#include <pthread.h>
#include <unistd.h>
#include "tcp_server.h"

#define BUFFER_SIZE 1024

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

IEC104_Obj iecProp;

TCP_Server tcp_pcb;


// Обрабтка ошибки
static void on_error(char *s, int *errCode)
{
    int err = (errCode) ? *errCode : WSAGetLastError();
    printf("%s: %d\n", s, err);
    exit(1);
}


// Прием TCP пакета
void TCP_OnReceived(TCP_Client *hcl, uint8_t *buf, uint16_t length)
{
    uint8_t txData[1024];
    IEC104_SetRxData(&iecProp, (uint8_t *) buf, length);
    IEC104_SetTxData(&iecProp, txData, sizeof(txData));
    IEC104_PacketHandler(&iecProp);
    TCP_Client_Send(hcl, iecProp.TxBuf.Data, iecProp.TxBuf.Len);
}


// Подключен новый клиент
void TCP_OnConnected(TCP_Client *hcl)
{

}

// Закрытие соединения
void TCP_OnClosed(TCP_Client *hcl)
{

}

// Периодическая функция обработки соединения
void TCP_PollCon(TCP_Client *hcl)
{
    uint8_t txData[1024];
    IEC104_SetTxData(&iecProp, txData, sizeof(txData));
    IEC104_SporadicPacket_Prepare(&iecProp);
    TCP_Client_Send(hcl, iecProp.TxBuf.Data, iecProp.TxBuf.Len);
}

// Основная программа
int main(int argc, char *argv[])
{
    WSADATA wsadata; 
    int port = 2404, err; 

    IEC104_Model_Init(&iecProp);

    err = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (err != 0)
        on_error("Errore in WSAStartup", &err);

    TCP_Init(&tcp_pcb, INADDR_ANY, port);

    while (1)
    {
        TCP_Handle(&tcp_pcb);
    }

    WSACleanup();
    return 0;
}