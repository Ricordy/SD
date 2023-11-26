/**
 * Grupo 58
 * Matheus Nunes 47883
 * Rodrigo Barrocas 53680
 */

#include "server_redundancy.h"
#include <string.h>
#include <unistd.h>

static u_int8_t is_connected; // Variável que indica se a conexão com o ZooKeeper está estabelecida
static zhandle_t *zh;         // Ponteiro para a estrutura que representa a conexão com o ZooKeeper

/**
 * Função de callback para monitorar o estado da conexão com o ZooKeeper
 */
void connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            is_connected = 1; // A conexão com o ZooKeeper foi estabelecida
        }
        else
        {
            is_connected = 0; // A conexão com o ZooKeeper foi perdida
        }
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            enum server_status *local_server_status = watcher_ctx;

            // Enivar o backup para primario
            if (*local_server_status == BACKUP && ZNONODE == zoo_exists(zh, "/kvstore/primary", 0, NULL))
            {
                void *buf = malloc(DATAMAXLEN);
                int buf_len = DATAMAXLEN;
                memset(buf, 0, buf_len);

                if (ZOK != zoo_get(zh, "/kvstore/backup", 0, buf, &buf_len, NULL))
                {
                    printf("Erro - Não foi possivel obter os dados do backup no zookeper. \n");
                }

                if (ZOK != zoo_create(zh, "/kvstore/primary", buf, buf_len, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0))
                {
                    printf("Erro - Não foi possivel criar o nó principal no zookeeper. \n");
                }

                free(buf);

                if (ZOK != zoo_delete(zh, "/kvstore/backup", -1))
                {
                    printf(" Erro - Não foi possivel deletar o nó do zookeeper. \n");
                }

                *local_server_status = PRIMARY_WITHOUT_BACKUP;
            }
            // Primário perdeu o seu backup
            if (*local_server_status == PRIMARY_WITH_BACKUP && ZNONODE == zoo_exists(zh, "/kvstore/backup", 0, NULL))
            {
                *local_server_status = PRIMARY_WITHOUT_BACKUP;
            }

            // Primário ganhoi o seu backup
            if (*local_server_status == PRIMARY_WITHOUT_BACKUP && ZOK == zoo_exists(zh, "/kvstore/backup", 0, NULL))
            {
                *local_server_status = PRIMARY_WITH_BACKUP;
            }
        }
    }
    if (ZOK != zoo_wget_children(zh, "/kvstore", &child_watcher, watcher_ctx, NULL))
    {
        printf("Error - Não foi possivel dar watch na /kvstore");
    }
}

/**
 * Inicializa a conexão com o ZooKeeper
 * @param zoo_host O endereço do servidor ZooKeeper
 * @return 0 se a conexão foi estabelecida com sucesso, -1 caso contrário
 */
int server_zoo_init(const char *zoo_host)
{
    if (is_connected)
        return 0; // Já está conectado ao ZooKeeper

    zh = zookeeper_init(zoo_host, connection_watcher, 2000, 0, 0, 0); // Inicializa a conexão com o ZooKeeper
    if (zh == NULL)
    {
        // Erro ao inicializar o ZooKeeper
        perror("Error- Não consegui inicializar o zookeper\n");
        return -1;
    }

    sleep(2); // Aguarda 2 segundos para a conexão ser estabelecida

    if (ZNONODE == zoo_exists(zh, "/kvstore", 0, NULL))
    {
        if (ZOK != zoo_create(zh, "/kvstore", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0))
        {
            perror("Error- Não consegui inicializar o zookeper");
        }
        void connection_watcher(zhandle_t * zh, int type, int state, const char *path, void *context)
        {
            if (type == ZOO_SESSION_EVENT)
            {
                if (state == ZOO_CONNECTED_STATE)
                {
                    is_connected = 1;
                }
                else
                {
                    is_connected = 0;
                }
            }
        }
    }
}

/**
 * Registra o servidor no ZooKeeper
 * @param data Os dados do servidor a serem registrados
 * @param datasize O tamanho dos dados
 * @return O status do registro do servidor
 */
enum server_status server_zoo_register(const char *data, size_t datasize)
{
    void connection_watcher(zhandle_t * zh, int type, int state, const char *path, void *context)
    {
        if (type == ZOO_SESSION_EVENT)
        {
            if (state == ZOO_CONNECTED_STATE)
            {
                is_connected = 1;
            }
            else
            {
                is_connected = 0;
            }
        }
    }
    if (!is_connected)
        return ERROR; // Não está conectado ao ZooKeeper

    struct String_vector children_list;       // Lista de nós filhos do nó "/kvstore"
    u_int8_t has_primary = 0, has_backup = 0; // Variáveis para verificar se já existem nós "primary" e "backup"

    enum ZOO_ERRORS log = zoo_get_children(zh, "/kvstore", 0, &children_list); // Obtém a lista de nós filhos

    if (ZOK != log)
    {
        printf("Erro - Não conseguiu encontrar children no zookeper.");
        return ERROR;
    }

    for (int i = 0; i < children_list.count; i++)
    {
        if (0 == strcmp(children_list.data[i], "backup"))
        {
            has_backup = 1; // Já existe um nó "backup"
        }
        if (0 == strcmp(children_list.data[i], "primary"))
        {
            has_primary = 1; // Já existe um nó "primary"
        }
    }

    if (!has_primary && !has_backup)
    {
        if (ZOK != zoo_create(zh, "/kvstore/primary", data, datasize, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0))
        {
            printf("Erro - Não foi possivel criar o node primario no zookeeper.");
            return ERROR;
        }
        return PRIMARY_WITH_BACKUP; // Registrado como "primary" com possibilidade de ter um "backup"
    }
    else if (has_primary && !has_backup)
    {
        log = zoo_create(zh, "/kvstore/backup", data, datasize, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
        if (ZOK != log)
        {
            printf("Erro - Não foi possivel criar um nó de backup no zookeeper.");
            return ERROR;
        }
    }
    else if (!has_primary && has_backup)
    {
        return REPEAT; // Já existe um "backup", não é possível registrar novamente
    }
    else
    {
        return NONE; // Já existe tanto "primary" quanto "backup"
    }
}

int server_zoo_setwatch(enum server_status *status)
{
    
}