#include "tcp_server.h"


#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif


#define BUFFER_SIZE 1024


// Прием TCP пакета
__weak void TCP_OnReceived(TCP_Client *hcl, uint8_t *buf, uint16_t length)
{
    UNUSED(hcl);
    UNUSED(buf);
    UNUSED(length);
}


// Подключен новый клиент
__weak void TCP_OnConnected(TCP_Client *hcl)
{
    UNUSED(hcl);
}

// Закрытие соединения
__weak void TCP_OnClosed(TCP_Client *hcl)
{
    UNUSED(hcl);
}

// Периодическая функция обработки соединения
__weak void TCP_PollCon(TCP_Client *hcl)
{
    UNUSED(hcl);
}

// Проверка статуса клиента
static bool TCP_IsConnected(SOCKET* sock)
{
    return ((*sock) != INVALID_SOCKET);
}


// Функция отправки данных
int TCP_Client_Send(TCP_Client* hcl, uint8_t *buf, int len)
{
    char *pbuf = (char *) buf;
    #if defined(_WIN32) || defined(_WIN64)
    int err;
    #endif
    do
    {
        int sent = send(hcl->Client_Fd, (char *) buf, len, 0);
        if (sent == SOCKET_ERROR)
        {
            #if defined(_WIN32) || defined(_WIN64)
            err = WSAGetLastError();
            if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                printf("Errore nella scrittura verso il client");
            #endif
            hcl->KeepLooping = false;
            break;
        }

        pbuf += sent;
        len -= sent;
    }
    while (len > 0);
    return 0;
}

// Обработчик задачи приема
static void* TCP_Receive_Task(void *args)
{
    TCP_Client *hcl = (TCP_Client *) args;
    char buf[BUFFER_SIZE];
    #if defined(_WIN32) || defined(_WIN64)
    int err;
    #endif
    hcl->KeepLooping = true;

    printf("Receive thread was started\r\n");

    do
    {
        int read = recv(hcl->Client_Fd, buf, BUFFER_SIZE, 0);

        pthread_mutex_lock(&hcl->Mutex);


        if (read == 0)
        {
            hcl->KeepLooping = false;
            pthread_mutex_unlock(&hcl->Mutex);
            break;
        }

        if (read == SOCKET_ERROR)
        {
            #if defined(_WIN32) || defined(_WIN64)
            err = WSAGetLastError();
            if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                printf("Errore nella lettura dal client");
            #endif
            break;
        }

        TCP_OnReceived(hcl, (uint8_t *) buf, read);


        pthread_mutex_unlock(&hcl->Mutex);
    }
    while (hcl->KeepLooping);
    #if defined(_WIN32) || defined(_WIN64)//Windows includes
    closesocket(hcl->Client_Fd);
    #else
    close(hcl->Client_Fd);
    #endif
    hcl->Client_Fd = INVALID_SOCKET;

    printf("Receive thread was stopped\r\n");

    return SUCCESS;
}

// Обработчик задачи периодического вызова
static void* TCP_Poll_Task(void *args)
{
    TCP_Client *hcl = (TCP_Client *) args;
    printf("Poll thread was started\r\n");

    while(TCP_IsConnected(&hcl->Client_Fd))
    {
        sleep(1);
        pthread_mutex_lock(&hcl->Mutex);
        TCP_PollCon(hcl);
        pthread_mutex_unlock(&hcl->Mutex);
    }

    TCP_OnClosed(hcl);
    pthread_mutex_destroy(&hcl->Mutex);
    free(hcl);
    printf("Connection closed\r\n");
    return SUCCESS;
}

// Инициализация сервера
int TCP_Init(TCP_Server *hs, uint32_t inadr, uint16_t port)
{
    int err;
    hs->Server_Fd = socket(AF_INET, SOCK_STREAM, 0);
    if (hs->Server_Fd == INVALID_SOCKET)
    {
        return -1;
    }

    memset(&hs->Server, 0, sizeof(hs->Server)); 
    hs->Server.sin_family = AF_INET; 
    hs->Server.sin_port = htons(port); 
    hs->Server.sin_addr.s_addr = inadr; 

        /** bind & listen **/
    const bool opt_val = true;
    setsockopt(hs->Server_Fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val));
    err = bind(hs->Server_Fd, (struct sockaddr *) &hs->Server, sizeof(hs->Server));
    if (err == SOCKET_ERROR)
        return -1;
    err = listen(hs->Server_Fd, 1);
    if (err == SOCKET_ERROR)
        return -1;

    return 0;
}


// Обработка сервера
static void TCP_Handle(TCP_Server *hs)
{
    int client_len = sizeof(struct sockaddr_in);
    TCP_Client *hcl = malloc(sizeof(TCP_Client));
    int status;

    if (hcl == NULL)
    {
        return;
    }

    pthread_mutex_init(&hcl->Mutex, NULL);

    hcl->Client_Fd = accept(hs->Server_Fd, (struct sockaddr *) &hcl->Client, (socklen_t *) &client_len);

    if (hcl->Client_Fd == INVALID_SOCKET)
        return;


    status = pthread_create(&hcl->ReceiveTask, NULL, TCP_Receive_Task, hcl);

    if (status != 0) {
        printf("main error: can't create thread, status = %d\n", status);
        exit(ERROR_CREATE_THREAD);
    }

    status = pthread_create(&hcl->PollTask, NULL, TCP_Poll_Task, hcl);


    if (status != 0) {
        printf("main error: can't create thread, status = %d\n", status);
        exit(ERROR_CREATE_THREAD);
    }

    TCP_OnConnected(hcl);   
}

// Задача приема TCP соединений
static void* TCP_Accept_Task(void *args)
{
    TCP_Server *hs = (TCP_Server *) args;

    while (hs->Active)
    {
        TCP_Handle(args);
    }

    return SUCCESS;
}

// Запуск TCP сервера
int TCP_Start(TCP_Server *hs)
{
    int status;
    hs->Active = true;
    status = pthread_create(&hs->Task, NULL, TCP_Accept_Task, hs);
    return status;
}


