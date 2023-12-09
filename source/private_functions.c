/* Grupo 16
 * Afonso Esteves 54394
 * Vicente Sousa 55386
 * João Anjos 54476
 */

#include "helper/priv-func.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void strapp(char **str, const char *app)
{
    *str = realloc(*str, (strlen(*str) + strlen(app) + 1) * sizeof(char));
    memcpy(*str + strlen(*str), app, (strlen(app) + 1) * sizeof(char));
}

int parse_address(const char *address_port, char **ip, short *port)
{
    size_t sz = -1;
    for (int i = 0; i < strlen(address_port); i++)
    {
        if (address_port[i] == ':')
        {
            *port = atoi(address_port + i + 1);
            sz = sizeof(char) * (i + 1);
            *ip = malloc(sz);
            memcpy(*ip, address_port, sizeof(char) * (i + 1));
            (*ip)[i] = '\0';
        }
    }
    return sz;
}

void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

void checkHostEntry(struct hostent *hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}

void myIp(char **buffer)
{
    char hostbuffer[256];
    ;
    struct hostent *host_entry;
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    *buffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
}