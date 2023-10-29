#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"

/* Remote table, que deve conter as informações necessária para comunicar*com o servidor. A definir pelo grupo em client_stub-private.h*/
struct rtable_t
{
    char *server_address;
    int server_port;
    int sockfd;
    struct sockaddr_in socket;
};

#endif