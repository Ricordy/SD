#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "inet.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


    /* Remote table, que deve conter as informações necessáriaspara comunicar*com o servidor. A definir pelo grupo em client_stub-private.h*/
struct rtable_t {
    char *server_address;
    int server_port;
    int sockfd;
    struct sockaddr_in socket;
};

#endif