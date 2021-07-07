#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdbool.h>
#include "iec104.h"
#include "iec104_model.h"
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

IEC104_Obj iecProp;
static SOCKET client_fd;

// Объект мьютекса
pthread_mutex_t mutex;


// Обрабтка ошибки
void on_error(char *s, int *errCode)
{
    int err = (errCode) ? *errCode : WSAGetLastError();
    printf("%s: %d\n", s, err);
    exit(1);
}

// Проверка статуса клиента
bool isConnected(SOCKET* sock)
{
    return ((*sock) != INVALID_SOCKET);
}

// Задача по бработке IEC104
void* iec104_handle(void *args)
{
    printf("Thread was started\r\n");

    while(isConnected(&client_fd))
    {

        uint8_t txData[1024];
        sleep(5);
        pthread_mutex_lock(&mutex);
        IEC104_SetTxData(&iecProp, txData, sizeof(txData));

        IEC104_SporadicPacket_Prepare(&iecProp);
        int sent = send(client_fd, (char *) iecProp.TxBuf.Data, iecProp.TxBuf.Len, 0);

         pthread_mutex_unlock(&mutex);
        if (sent == SOCKET_ERROR)
        {
            printf("Transmition error\r\n");
            break;
        }
    }

    printf("Connection closed\r\n");

    return SUCCESS;
}

int main(int argc, char *argv[])
{
    WSADATA wsadata; 
    SOCKET server_fd;
    struct sockaddr_in server, client;
    int port = 2404, err; 
    char buf[BUFFER_SIZE];

    pthread_t thread;
	int status;

    pthread_mutex_init(&mutex, NULL);

    IEC104_Model_Init(&iecProp);

    err = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (err != 0)
        on_error("Errore in WSAStartup", &err);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
        on_error("Non ho potuto creare il socket", NULL);

    memset(&server, 0, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = INADDR_ANY; 

    /** bind & listen **/
    const BOOL opt_val = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val));
    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err == SOCKET_ERROR)
        on_error("Non ho potuto fare il bind del socket", NULL);
    err = listen(server_fd, 1);
    if (err == SOCKET_ERROR)
        on_error("Non ho potuto mettermi in ascolto sul socket", NULL);

    printf("SERVER LISTENING ON PORT %d\n", port);

    while (1)
    {
        int client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd == INVALID_SOCKET)
            on_error("Non riesco a stabilire una nuova connessione", NULL);

        status = pthread_create(&thread, NULL, iec104_handle, NULL);

        if (status != 0) {
            printf("main error: can't create thread, status = %d\n", status);
            exit(ERROR_CREATE_THREAD);
        }

        bool keepLooping = true;
        do
        {
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);

            uint8_t txData[1024];

            pthread_mutex_lock(&mutex);
            IEC104_SetRxData(&iecProp, (uint8_t *) buf, read);
            IEC104_SetTxData(&iecProp, txData, sizeof(txData));
            IEC104_PacketHandler(&iecProp);

            if (read == 0)
            {
                keepLooping = false;
                pthread_mutex_unlock(&mutex);
                break;
            }

            if (read == SOCKET_ERROR)
            {
                err = WSAGetLastError();
                if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                    on_error("Errore nella lettura dal client", &err);
                break;
            }

            char *pbuf = buf;
            do
            {
                int sent = send(client_fd, (char *) iecProp.TxBuf.Data, iecProp.TxBuf.Len, 0);
                if (sent == SOCKET_ERROR)
                {
                    err = WSAGetLastError();
                    if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                        on_error("Errore nella scrittura verso il client", &err);

                    keepLooping = false;
                    break;
                }

                pbuf += sent;
                read -= sent;
            }
            while (read > 0);
            pthread_mutex_unlock(&mutex);
        }
        while (keepLooping);

        closesocket(client_fd);
        IEC104_Model_Init(&iecProp);
        client_fd = INVALID_SOCKET;
    }

    WSACleanup();
    return 0;
}