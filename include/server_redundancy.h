/**
 * Grupo 58
 * Matheus Nunes 47883
 * Rodrigo Barrocas 53680
 */

#ifndef _SERVER_REDUNDANCY_H
#define _SERVER_REDUNDANCY_H

#include <zookeeper/zookeeper.h>

#define ZDATALEN 1024 * 1024
#define DATAMAXLEN 22 // 255.255.255.255:65535

enum server_status
{
    ERROR = -1,
    NONE,
    PRIMARY_WITH_BACKUP,
    PRIMARY_WITHOUT_BACKUP,
    BACKUP,
    REPEAT
};
/*
ERROR - Server retornou com erro
NONE - Esta conexão será recusada pela three
PRIMARY_WITH_BACKUP - Este servidor é primário e tem, um servidor backup, o pedido do cliente será reenviado
PRIMARY_WITHOUT_BACKUP - Este servidor é primário e não tem servidor backup, o pedido do cliente será recusado
BACKUP - Servidor de backup
REPEAT - O servidor tentará registar-se novamente após um periodo definido
*/

int server_zoo_init(const char *zoo_host);

enum server_status server_zoo_register(const char *data, size_t datasize);

int server_zoo_setwatch(enum server_status *status);

int server_zoo_get_primary(char *meta_data, int size);

int server_zoo_get_backup(char *meta_data, int size);

void server_zoo_close();

#endif