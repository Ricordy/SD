#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "sdmessage.pb-c.h"

struct message_t
{
    MessageT *msgConvert;
};

int write_all(int socket, void *buffer, int size);

int read_all(int socket, void *buffer, int size);

#endif