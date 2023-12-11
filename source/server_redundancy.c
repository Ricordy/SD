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

char *zkServerNodePath = NULL;
char *zkServerNodeID = NULL;

static char *watcher_ctx = "ZooKeeper Data Watcher";
struct rtable_t *remote_server_table;

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
            printf("9. status/watcher_ctx:  %s \n", watcher_ctx);

            if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_ctx, children_list))
            {
                fprintf(stderr, "Erro a configurar watch no %s!\n", "/chain");
                exit(1);
            }

            sortNodeIds(children_list);
            printf("nodeId pré chamada: %s\n", zkServerNodeID);
            snet.proximo_node = getNextNode(children_list, zkServerNodeID);
            printf("9.1 proximo node:  %s \n", snet.proximo_node);
            if (snet.proximo_node == NULL)
            {
                if (snet.proximo_node_path != NULL)
                {
                    free(snet.proximo_node_path);
                }
                snet.proximo_node_path = NULL;
                printf("PRoximo nó: Nó final.");
            }
            else
            {
                printf("9.2 proximo node:  %s \n", snet.proximo_node);
                snet.proximo_node_path = malloc((7 + strlen(snet.proximo_node) + 1) * sizeof(char));
                strcpy(snet.proximo_node_path, "/chain/");
                strcat(snet.proximo_node_path, snet.proximo_node);
                printf("Next node: %s\n", snet.proximo_node);
                printf("Next node path: %s\n", snet.proximo_node_path);

                int buffer_len = 1000;
                printf("9.2.1 buffer_len:  %d \n", buffer_len);
                char *buffer = malloc(1000);
                if (buffer == NULL)
                {
                    fprintf(stderr, "Erro, falha na reserva de memoria");
                    exit(1);
                }
                printf("9.3 \n");
                if (ZOK != zoo_get(zh, snet.proximo_node_path, 0, buffer, &buffer_len, NULL))
                {
                    printf("Não foi possiverl obter a metadata do node: %s\n", snet.proximo_node_path);
                    free(buffer);
                    exit(1);
                }
                if (snet.proximo_server_add != NULL)
                {
                    printf("9.3.1\n");
                    free(snet.proximo_server_add);
                }
                printf("9.3A\n");
                snet.proximo_server_add = malloc(strlen(buffer) + 1);
                strcpy(snet.proximo_server_add, buffer);
                free(buffer);
                printf("9.4\n");
                // Conectar ao proximo node
                if (snet.next_table == NULL)
                {
                    printf("9.4.1\n");
                    printf("A conectar ao proximo node no servidor %s... \n", snet.proximo_server_add);
                    snet.next_table = rtable_connect(snet.proximo_server_add);
                }
                else
                {
                    printf("9.4.2\n");
                    char *str = malloc((strlen(snet.next_table->server_address) + 1 + strlen(snet.next_table->server_port) + 1) * sizeof(char));
                    strcpy(str, snet.next_table->server_address);
                    strcat(str, "");
                    strcat(str, snet.next_table->server_port);
                    if (strcmp(str, snet.proximo_server_add) != 0)
                    {
                        if (snet.next_table != NULL)
                        {
                            free(snet.next_table->server_address);
                            free(snet.next_table->server_port);
                            free(snet.next_table);
                        }
                        snet.next_table = connect_zoo_server(remote_server_table, snet.proximo_server_add);
                        printf("Proximo serviudor alterado, nova conexão feita! \n");
                    }
                }
                printf("9.5\n");

                printf("Endereço do proximo servidor %s\n", snet.proximo_server_add);
                // printf("IP do proximo servidor %s\n", snet.next_table->server_address);
                // printf("Porto do proximo servidor %d\n", snet.next_table->server_port);
                // printf("Socket do proximo servidor %d\n", snet.next_table->sockfd);
            }
            fprintf(stderr, "\n------------ Node em espera ------------\n");
            for (int i = 0; i < children_list->count; i++)
            {
                fprintf(stderr, "Filho n %d, data: %s \n", i + 1, children_list->data[i]);
            }
            fprintf(stderr, "\n------------ Feito ------------\n");
            // if (*local_server_status == SUCCESS && ZOK == zoo_exists(zh, "/chain/backup", 0, NULL))
            // {
            //     *local_server_status = SUCCESS;
            // }
        }
    }
    free(children_list);
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

    sleep(4); // Aguarda 2 segundos para a conexão ser estabelecida

    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL))
    {
        fprintf(stderr, "/chain não existe, a criar...\n");
        fflush(stderr);
        if (ZOK != zoo_create(zh, "/chain", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0))
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
    sleep(4); // Aguarda 2 segundos para a conexão ser estabelecida

    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL))
    {
        fprintf(stderr, "/chain não existe, a criar...\n");
        fflush(stderr);
        if (ZOK != zoo_create(zh, "/chain", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0))
        {
            perror("Error- Não consegui inicializar o zookeper");
            return -1;
        }
    }

    int path_len = 1024;
    char *new_path = malloc(path_len);
    if (new_path == NULL)
    {
        printf("Erro - Não fpoi possivel reservar a memoria.\n");
        return ERROR;
    }
    printf("A criar child node /chain/node... \n");
    fflush(stdout);

    printf("6.\n");
    printf("6.1 data/this_ip_port:  %s\n", data);

    printf("6.2 ZOK: %d\n", ZOK);
    int zoo_status = zoo_create(zh, "/chain/node", data, datasize, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, path_len);
    sleep(2);
    if (ZOK != zoo_status)
    {
        if (ZNONODE == zoo_status)
            printf("ZNONDE!\n");
        fprintf(stderr, "Erro - Não foi possivel criar o node no zookeeper. Error code: %d %d\n", ZOK, zoo_status);
        return ERROR;
    }
    zkServerNodePath = new_path;
    zkServerNodeID = zkServerNodePath + strlen("/chain/");
    printf("Nó de sequencia efemera criado! Path do ZNode: %s\n", new_path);
    // free(new_path);

    return SUCCESS; // Registrado como "primary" com possibilidade de ter um "backup"
}

int server_zoo_setwatch(enum server_status *status)
{
    if (!is_connected)
        return -1;

    printf("7\n");
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string)); // Lista de nós filhos do nó "/chain"
    if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_ctx, children_list))
    {
        free(children_list);
        printf("ERRO - Não foi possivel iniciar a watch\n");
        return -1;
    } // Obtém a lista de nós filhos
    free(children_list);
    return 0;
}

int connect_zoo_server(struct rtable_t *server, char *serverInfo)
{
    server = malloc(sizeof(struct rtable_t));
    printf("SERVER INFO: %s\n\n", serverInfo);
    // char *host = strtok((char *)serverInfo, ":"); // hostname    removed:
    // int port = atoi(strtok(NULL, ":"));           // port      '<' ':' '>'
    printf("9.4.1.1 \n");
    for (int i = 0; i < strlen(serverInfo); i++) // Percorrer o array de char fornecido
    {
        if (serverInfo[i] == ':') // quando encontramos a divisão ip:porto
        {
            printf("9.4.1.1.1 \n");
            server->server_port = atoi(serverInfo + i + 1);
            printf("9.4.1.1.1 port done \n");                        // Converter a string relativa ao porto
            server->server_address = malloc(sizeof(char) * (i + 1)); // Reservar o espaço necessário para o ip do servidor
            printf("9.4.1.1.2 \n");
            memcpy(server->server_address, serverInfo, sizeof(char) * (i + 1)); // copiar o valor do ip para a o endereço da memoria que definimos anteriormente
            ((char *)server->server_address)[i] = '\0';
            printf("9.4.1.1.3 \n");
            server->socket.sin_family = AF_INET;
            server->socket.sin_port = htons(server->server_port); // Colocar o caracter de fim de string
            printf("9.4.1.1.4 \n");
        }
    }

    printf("9.4.1.2\n");
    if (inet_pton(AF_INET, server->server_address, &server->socket.sin_addr) < 1)
    {
        printf("Erro ao converter IP\n");
        return -1;
    }
    // Criar socket TCP
    if ((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro ao criar socket TCP - Cliente");
        return -1;
    }
    // printf("SOCKET NUMBER: %d\n\n\n", server->socket_num);

    // Estabelece conexao com o servidor
    if (connect(server->sockfd, (struct sockaddr *)&server->socket, sizeof(server->socket)) < 0)
    {
        perror("Erro ao conetar ao servidor - Client");
        close(server->sockfd);
        return -1;
    }

    // signal(SIGPIPE, conn_lost);

    printf("Conetado ao próximo servidor\n");

    return 0;
}

int server_zoo_get_primary(char *meta_data, int size)
{
    if (!is_connected)
        return -1;
    memset(meta_data, 0, size);
    if (ZOK != zoo_get(zh, "/chain/node", 0, meta_data, &size, NULL))
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
    if (ZOK != zoo_get(zh, "/chain/node", 0, meta_data, &size, NULL))
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