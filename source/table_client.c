/**
 * Rodrigo Barrocas - 53680
 * Matheus Nunes -
 * Pedro Vaz -
 */

#include <signal.h>
#include "network_client.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "inet.h"
#include "stats.h"

#include <zookeeper/zookeeper.h>

typedef struct String_vector zoo_string;

char *zookeeperAddress;
static zhandle_t *zh;
static int is_connected;
static char *watcher_ctx = "ZooKeeper Data Watcher";

char *head_address = NULL, *tail_address = NULL;
struct rtable_t *rtable_head = NULL, *rtable_tail = NULL;

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

char *getNodeMetaData(zhandle_t *zh, const char *nodeId)
{
    char *nodePath = malloc((7 + strlen(nodeId) + 1) * sizeof(char));
    strcpy(nodePath, "/chain/");
    strcat(nodePath, nodeId); // /chain/node0000...

    int buffer_len = 1000;
    char *buffer = malloc(1000);
    if (ZOK != zoo_get(zh, nodePath, 0, buffer, &buffer_len, NULL))
    {
        printf("Error getting metadata from %s\n", nodePath);
        exit(EXIT_FAILURE);
    }
    char *server_address = malloc(strlen(buffer));
    strcpy(server_address, buffer);
    free(buffer);
    return server_address;
}

/**
 * Watcher function for connection state change events
 */
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context)
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

/**
 * Data Watcher function for this node
 * To be run if there is a change in /chain children
 */
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    // int zoo_data_len = ZDATALEN;
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(zh, "/chain", child_watcher, watcher_ctx, children_list))
            {
                fprintf(stderr, "Error setting watch at %s!\n", "/chain");
            }

            sortNodeIds(children_list);
            char *zk_head = children_list->data[0];
            char *zk_tail = children_list->data[children_list->count - 1];
            char *new_head_address = getNodeMetaData(zh, zk_head);
            char *new_tail_address = getNodeMetaData(zh, zk_tail);

            if (head_address == NULL)
            {
                head_address = strdup(new_head_address);
                rtable_head = rtable_connect(head_address);
            }
            else
            {
                free(head_address);
                free(rtable_head->server_address);
                free(rtable_head->server_port);
                free(rtable_head);
                head_address = strdup(new_head_address);
                rtable_head = rtable_connect(head_address);
                printf("Head server changed! New connection established.");
            }

            if (rtable_head == NULL)
            {
                printf("ERROR: could not connect to HEAD server.\n");
                exit(1);
            }

            if (tail_address == NULL)
            {
                tail_address = strdup(new_tail_address);
                rtable_tail = rtable_connect(tail_address);
            }
            else
            {
                free(tail_address);
                free(rtable_tail->server_address);
                free(rtable_tail->server_port);
                free(rtable_tail);
                tail_address = strdup(new_tail_address);
                rtable_tail = rtable_connect(tail_address);
                printf("Tail server changed! New connection established.");
            }

            if (rtable_tail == NULL)
            {
                printf("ERROR: could not connect to TAIL server.\n");
                exit(1);
            }

            fprintf(stdout, "\n=== znode listing === [ %s ]", "/chain");
            for (int i = 0; i < children_list->count; i++)
            {
                fprintf(stdout, "\n(%d): %s", i + 1, children_list->data[i]);
            }
            fprintf(stdout, "\n=== done ===\n");
        }
    }
    free(children_list);
}

int main(int argc, char **argv)
{
    // Criação de variaveis auxilares
    char comando[MAX_MSG];         // Guarda todo a frase do comando
    const char divisor[3] = " \n"; // Array usado para divisão
    char *operacao;                // Guarda operacao
    char *key;                     // Guarda chave da tabela
    char *dados;                   // Guarda dados a modificar

    // Verificar argumentos
    if (argc != 2 || argc < 0)
    {
        printf("Forma correta de inicar o programa: ./table-client <server>:<port>\n");
        return -1;
    }
    // setup connection to zookeeper
    zookeeperAddress = argv[1];
    zh = zookeeper_init(zookeeperAddress, connection_watcher, 2000, 0, 0, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        exit(EXIT_FAILURE);
    }
    sleep(3); /* Sleep a little for connection to complete */

    if (is_connected)
    {
        // SETUP WATCH FOR /chain CHILDREN
        zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
        if (ZOK != zoo_wget_children(zh, "/chain", child_watcher, watcher_ctx, children_list))
        {
            fprintf(stderr, "Error setting watch at %s!\n", "/chain");
        }

        sortNodeIds(children_list);
        char *zk_head = children_list->data[0];
        char *zk_tail = children_list->data[children_list->count - 1];
        char *new_head_address = getNodeMetaData(zh, zk_head);
        char *new_tail_address = getNodeMetaData(zh, zk_tail);

        if (head_address == NULL)
        {
            head_address = strdup(new_head_address);
            rtable_head = rtable_connect(head_address);
        }
        else
        {
            free(head_address);
            free(rtable_head->server_address);
            free(rtable_head->server_port);
            free(rtable_head);
            head_address = strdup(new_head_address);
            rtable_head = rtable_connect(head_address);
            printf("Head server changed! New connection established.");
        }

        if (rtable_head == NULL)
        {
            printf("ERROR: could not connect to HEAD server.\n");
            exit(1);
        }

        if (tail_address == NULL)
        {
            tail_address = strdup(new_tail_address);
            rtable_tail = rtable_connect(tail_address);
        }
        else
        {
            free(tail_address);
            free(rtable_tail->server_address);
            free(rtable_tail->server_port);
            free(rtable_tail);
            tail_address = strdup(new_tail_address);
            rtable_tail = rtable_connect(tail_address);
            printf("Tail server changed! New connection established.");
        }

        if (rtable_tail == NULL)
        {
            printf("ERROR: could not connect to TAIL server.\n");
            exit(1);
        }

        fprintf(stdout, "\n=== znode listing === [ %s ]", "/chain");
        for (int i = 0; i < children_list->count; i++)
        {
            fprintf(stdout, "\n(%d): %s", i + 1, children_list->data[i]);
        }
        fprintf(stdout, "\n=== done ===\n");
    }

    // Cicle de funcionamento do cliente
    while (1)
    {
        printf("Introduza uma acao: ");
        if (fgets(comando, MAX_MSG, stdin) == NULL) // Verificar se a acao existe
        {
            printf("Nao introduziu nenhum comando!\n");
            break;
        }

        operacao = strtok(comando, divisor);
        // Comando invalido
        if (operacao == 0)
        {
            printf("Introduza um comando valido!\n");
            printf("Lista de comandos validos:\n");
            printf(" size\n");
            printf(" del <key>\n");
            printf(" get <key>\n");
            printf(" put <key> <data>\n");
            printf(" getkeys\n");
            printf(" stats\n");
            printf(" quit\n");
        }
        // Comando valido
        else
        {
            if (strcmp(operacao, "quit") == 0) // Comando QUIT
            {
                printf("A terminar a ligacao...\n");
                rtable_disconnect(rtable_tail);
                rtable_disconnect(rtable_head);
                break;
            }
            else if ((strcmp(operacao, "size") == 0)) // Comando SIZE
            {
                int tamanho = rtable_size(rtable_tail);
                printf("Tamanho -> %d\n", tamanho);
            }
            else if (strcmp(operacao, "del") == 0) // Comando DEL
            {
                key = strtok(NULL, divisor);
                if (key == NULL) // Verificar se foi passada uma key no comando
                {
                    printf("Este comando necessita de mais um parametro: del <key>");
                }
                else // Caso em que a key foi passada
                {
                    if (rtable_del(rtable_head, key) == -1) // Eliminar e verificar o resultado da operação
                    {
                        printf("Erro a apagar, verifique a existencia da <key> na tabela\n"); // Mensagem de erro
                    }
                    else
                    {
                        printf("Eliminado com sucesso!\n"); // Mensagem de sucesso
                    }
                }
            }
            else if (strcmp(operacao, "get") == 0) // Comando GET
            {
                key = strtok(NULL, divisor);
                if (key == NULL) // Verificar se foi passada uma key no comando
                {
                    printf("Este comando necessita de mais um parametro: get <key>");
                }
                else
                {
                    struct data_t *dadosRecebidos;                               // Variavel de apoio para guardar dados/erro
                    if ((dadosRecebidos = rtable_get(rtable_tail, key)) == NULL) // Verificação da existencia dos dados na tabela e atribuição do valor à variavel
                    {
                        printf("Erro a obter o valor, key invalida!\n"); // Mensagem de erro
                    }
                    else
                    {
                        printf("Dados obtidos com sucesso\n");
                        printf("Dados: %s \n", (char *)dadosRecebidos->data);
                        data_destroy(dadosRecebidos); // Destruir variavel auxiliar
                    }
                }
            }
            else if (strcmp(operacao, "put") == 0) // Comando PUT
            {
                key = strtok(NULL, divisor); // Obter o valor da key
                dados = strtok(NULL, "\n");  // Obter o valor dos dados
                if (dados == NULL)
                {
                    printf("O comando necessita de mais um parametro, put <key> <data>\n");
                }
                else
                {
                    struct data_t *data = data_create(strlen(dados) + 1, dados);
                    struct entry_t *entry = entry_create(key, data);
                    if (rtable_put(rtable_head, entry) == -1)
                    {
                        printf("Erro a colocar os dados na tabela!");
                    }
                    else
                    {
                        printf("Dados colocados com sucesso!");
                    }
                    free(data);
                    free(entry);
                }
            }
            else if (strcmp(operacao, "getkeys") == 0) // Comando GETKEYS
            {
                char **keys = rtable_get_keys(rtable_tail); // Criação de variavel de apoio
                printf("vou imprimir keys: \n");
                int index;                                       // Variavel de apoio
                for (index = 0; keys[index] != NULL; index += 1) // Ciclio para imprimir as keys na tela
                {
                    printf("tentei %d \n", index);
                    printf("%s \n", keys[index]);
                }
            }
            else if (strcmp(operacao, "stats") == 0) // Comando STATS
            {
                struct statistics_t *stats = rtable_stats(rtable_tail); // Criação de variavel de apoio
                if (stats == NULL)
                {
                    printf("Erro a obter as estatisticas!\n"); // Mensagem de erro
                }
                else
                {
                    printf("Numero total de operacoes: %d\n", stats->total_operations);
                    printf("Numero de clientes ativos: %d\n", stats->connected_clients);
                    printf("Tempo total acumulado nas execuções: %ld\n", stats->total_time);
                    free(stats);
                }
            }
            else // Comando intruduzido é invalido
            {
                printf("Introduza um comando valido!\n");
                printf("Lista de comandos validos:\n");
                printf(" size\n");
                printf(" del <key>\n");
                printf(" get <key>\n");
                printf(" put <key> <data>\n");
                printf(" getkeys\n");
                printf(" stats\n");
                printf(" quit\n");
            }
        }
    }
}