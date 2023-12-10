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

typedef struct String_vector zoo_string;
struct server_net_t snet = {0};

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
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            enum server_status *local_server_status = watcher_ctx;

            if (!ZOK != zoo_wget_children(zh, "/chain", child_watcher, local_server_status, children_list))
            {
            }

            // Enivar o backup para primario
            // if (*local_server_status == SUCCESS && ZNONODE == zoo_exists(zh, "/chain/primary", 0, NULL))
            // {
            //     void *buf = malloc(DATAMAXLEN);
            //     int buf_len = DATAMAXLEN;
            //     memset(buf, 0, buf_len);

            //     if (ZOK != zoo_get(zh, "/chain/backup", 0, buf, &buf_len, NULL))
            //     {
            //         printf("Erro - Não foi possivel obter os dados do backup no zookeper. \n");
            //     }

            //     if (ZOK != zoo_create(zh, "/chain/primary", buf, buf_len, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0))
            //     {
            //         printf("Erro - Não foi possivel criar o nó principal no zookeeper. \n");
            //     }

            //     free(buf);

            //     if (ZOK != zoo_delete(zh, "/chain/backup", -1))
            //     {
            //         printf(" Erro - Não foi possivel deletar o nó do zookeeper. \n");
            //     }

            //     *local_server_status = SUCCESS;
            // }
            // // Primário perdeu o seu backup
            // if (*local_server_status == SUCCESS && ZNONODE == zoo_exists(zh, "/chain/backup", 0, NULL))
            // {
            //     *local_server_status = SUCCESS;
            // }

            // // Primário ganhoi o seu backup
            // if (*local_server_status == SUCCESS && ZOK == zoo_exists(zh, "/chain/backup", 0, NULL))
            // {
            //     *local_server_status = SUCCESS;
            // }
        }
    }
    if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_ctx, NULL))
    {
        printf("Error - Não foi possivel dar watch na /chain");
    }
}

/**
 * Inicializa a conexão com o ZooKeeper
 * @param zoo_host O endereço do servidor ZooKeeper
 * @return 0 se a conexão foi estabelecida com sucesso, -1 caso contrário
 */
int server_zoo_init(const char *zoo_host)
{
    printf("Entrei no server_zoo_init \n");
    if (is_connected)
    {
        printf("Ja está conectado ao zookeper. \n");
        return 0; // Já está conectado ao ZooKeeper
    }

    zh = zookeeper_init(zoo_host, connection_watcher, 2000, 0, 0, 0); // Inicializa a conexão com o ZooKeeper
    if (zh == NULL)
    {
        // Erro ao inicializar o ZooKeeper
        perror("Error- Não consegui inicializar o zookeper\n");
        return -1;
    }

    printf("ZNONODE: %d  %d %d  \n", ZNONODE, ZOK, ZOO_CONNECTED_STATE);

    sleep(3); // Aguarda 2 segundos para a conexão ser estabelecida

    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL))
    {
        fprintf(stderr, "/chain não existe, a criar...\n");
        fflush(stderr);
        if (ZOK != zoo_create(zh, "/chain", "chain node", 11, &ZOO_OPEN_ACL_UNSAFE, 2, NULL, 0))
        {
            perror("Error- Não consegui inicializar o zookeper");
            return -1;
        }
    }

    return 0;
}

/**
 * Registra o servidor no ZooKeeper
 * @param data Os dados do servidor a serem registrados
 * @param datasize O tamanho dos dados
 * @return O status do registro do servidor
 */
enum server_status server_zoo_register(const char *data, size_t datasize)
{
    if (!is_connected)
    {
        printf("Não está conectado ao zookeeper, não foi possivel inicar o node.\n");
        return ERROR; // Não está conectado ao ZooKeeper
    }
    int path_len = 1024;
    char *new_path = malloc(path_len);
    printf("A criar child node /chain/node... \n");
    fflush(stdout);

    printf("6.\n");

    printf("6.2\n");

    if (ZOK != zoo_create(zh, "/chain/node", data, datasize, &ZOO_OPEN_ACL_UNSAFE, 3, new_path, path_len))
    {
        printf("Erro - Não foi possivel criar o node primario no zookeeper.");
        return ERROR;
    }
    printf("Nó de sequencia efemera criado! Path do ZNode: %s\n", new_path);
    return SUCCESS; // Registrado como "primary" com possibilidade de ter um "backup"
}

int server_zoo_setwatch(enum server_status *status)
{
    if (!is_connected)
        return -1;

    printf("7\n");
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string)); // Lista de nós filhos do nó "/chain"
    if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, status, children_list))
    {
        printf("ERRO - Não foi possivel iniciar a watch\n");
        return -1;
    } // Obtém a lista de nós filhos

    return 0;
}

int server_zoo_get_primary(char *meta_data, int size)
{
    if (!is_connected)
        return -1;
    memset(meta_data, 0, size);
    if (ZOK != zoo_get(zh, "/chain/primary", 0, meta_data, &size, NULL))
    {
        printf("Error! - Couldn't get data at primary in zookeeper");
        return -1;
    }
    return size;
}

int server_zoo_get_backup(char *meta_data, int size)
{
    if (!is_connected)
        return -1;
    memset(meta_data, 0, size);
    if (ZOK != zoo_get(zh, "/chain/backup", 0, meta_data, &size, NULL))
    {
        printf("Error! - Couldn't get data at backup in zookeeper");
        return -1;
    }
    return size;
}

void server_zoo_close()
{
    if (is_connected == 1)
    {
        zookeeper_close(zh);
        is_connected = 0;
    }
}