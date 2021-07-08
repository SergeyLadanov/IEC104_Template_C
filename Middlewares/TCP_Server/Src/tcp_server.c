#include "tcp_server.h"


#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif


// Прием TCP пакета
__weak TCP_OnReceived(SOCKET *cl, uint8_t *buf, uint16_t length)
{
    UNUSED(cl);
    UNUSED(buf);
    UNUSED(length);
}


// Подключен новый клиент
__weak TCP_OnConnected(SOCKET *cl)
{
    UNUSED(cl);
    UNUSED(buf);
    UNUSED(length);
}

// Закрытие соединения
__weak TCP_OnClosed(SOCKET *cl)
{
    UNUSED(cl);
    UNUSED(buf);
    UNUSED(length);
}

// Периодическая функция обработки соединения
__weak TCP_PollCon(SOCKET *cl)
{
    UNUSED(cl);
    UNUSED(buf);
    UNUSED(length);
}

// Проверка статуса клиента
bool TCP_IsConnected(SOCKET* sock)
{
    return ((*sock) != INVALID_SOCKET);
}


// Инициализация сервера
int TCP_Init(TCP_Server *hs, in_addr_t inadr, uint16_t port)
{
    hs->Server_Fd = socket(AF_INET, SOCK_STREAM, 0);
    if (Server_Fd == INVALID_SOCKET)
    {
        return -1;
    }

    memset(&hs->Server, 0, sizeof(hs->Server)); 
    hs->Server.sin_family = AF_INET; 
    hs->Server.sin_port = htons(port); 
    hs->Server.sin_addr.s_addr = inadr; 

        /** bind & listen **/
    const BOOL opt_val = TRUE;
    setsockopt(hs->Server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val));
    err = bind(hs->Server_fd, (struct sockaddr *) &hs->Server, sizeof(hs->Server));
    if (err == SOCKET_ERROR)
        return -1;
    err = listen(hs->Server_fd, 1);
    if (err == SOCKET_ERROR)
        return -1;

    return 0;
}


// Обработка сервера
void TCP_Handle(TCP_Server *hs)
{

}