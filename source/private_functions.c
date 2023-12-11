/* Grupo 58
 * Rodrigo Barrocas 53680
 * Matheus Nunes 47883
 */

#include "private_functions.h"
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

void strappend(char **str, const char *app)
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

void sortNodeIds(zoo_string *idList)
{
    int not_sorted = 1;
    char *tmp;

    while (not_sorted)
    {
        not_sorted = 0;
        for (int i = 0; i < idList->count - 1; i++)
        {
            for (int j = i + 1; j < idList->count; j++)
            {
                if (strcmp(idList->data[i], idList->data[j]) > 0)
                {
                    not_sorted = 1;
                    tmp = idList->data[j];
                    idList->data[i] = idList->data[j];
                    idList->data[j] = tmp;
                }
            }
        }
    }
}

char *getNextNode(zoo_string *idList, char *nodeId)
{
    printf("Arguments:\n");
    printf("idList->count: %d\n", idList->count);
    printf("nodeId: %s\n", nodeId);

    for (int i = 0; i < idList->count; i++)
    {
        if (strcmp(idList->data[i], nodeId) == 0)
        {
            if (i == idList->count - 1)
            {
                // end of list
                printf("End of list reached. Returning NULL.\n");
                return NULL;
            }

            printf("Node found in the list. Returning the next node.\n");
            printf("Next node: %s\n", idList->data[i + 1]);
            return idList->data[i + 1];
        }
    }

    // Node not found in the list
    printf("Error: Node id not found in the list!\n");
    printf("Node Id: %s\n", nodeId);
    printf("\nList Contents:\n");
    for (int i = 0; i < idList->count; i++)
    {
        printf("(%d): %s\n", i + 1, idList->data[i]);
    }
    printf("\n");
    exit(EXIT_FAILURE);
}