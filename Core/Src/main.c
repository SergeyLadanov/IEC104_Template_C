#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdbool.h>
#include "iec104.h"
#include "iec104_model.h"
#include <pthread.h>
#include <unistd.h>
#include "tcp_server.h"

// Объект TCP сервера
TCP_Server tcp_pcb;

// Вспомогательная структура
typedef struct __IEC_Con
{
    IEC104_Obj *IecProp;
    uint32_t DelayCnt;
} IEC_Con;


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
    IEC_Con *hcon = (IEC_Con *) hcl->Arg;
    uint8_t txData[1024];
    IEC104_SetRxData(hcon->IecProp, (uint8_t *) buf, length);
    IEC104_SetTxData(hcon->IecProp, txData, sizeof(txData));
    IEC104_PacketHandler(hcon->IecProp);
    TCP_Client_Send(hcl, hcon->IecProp->TxBuf.Data, hcon->IecProp->TxBuf.Len);
}


// Подключен новый клиент
void TCP_OnConnected(TCP_Client *hcl)
{
    IEC_Con *hcon = malloc(sizeof(IEC_Con));
    
    if (hcon == NULL)
    {
        printf("Error of iec104 memory allocation");
    }

    hcon->DelayCnt = 0;

    hcon->IecProp = malloc(sizeof(IEC104_Obj));

    if (hcon->IecProp == NULL)
    {
        printf("Error of iec104 memory allocation");
    }
    IEC104_Model_Init(hcon->IecProp);

    // Пример записи значения в модель IEC104
    IEC104_SetFloat(hcon->IecProp, 1, 8204, 0.1234f);
    
    hcl->Arg = hcon;
}

// Закрытие соединения
void TCP_OnClosed(TCP_Client *hcl)
{
    IEC_Con *hcon = (IEC_Con *) hcl->Arg;
    IEC104_Model_Init(hcon->IecProp);
    free(hcon);
}

// Периодическая функция обработки соединения
void TCP_PollCon(TCP_Client *hcl)
{
    IEC_Con *hcon = (IEC_Con *) hcl->Arg;

    if (hcon->DelayCnt++ > 5)
    {
        uint8_t txData[1024];
        IEC104_SetTxData(hcon->IecProp, txData, sizeof(txData));
        IEC104_SporadicPacket_Prepare(hcon->IecProp);
        TCP_Client_Send(hcl, hcon->IecProp->TxBuf.Data, hcon->IecProp->TxBuf.Len);
        hcon->DelayCnt = 0;
    }

}

// Основная программа
int main(int argc, char *argv[])
{
    WSADATA wsadata; 
    int port = 2404, err; 

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