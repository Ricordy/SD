#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

struct message_t
{
    struct _MessageT *msgConvert;
};

int write_all(int socket, void *buffer, int size);

int read_all(int socket, void *buffer, int size);

#endif