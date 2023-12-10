#include "table.h"
#include "inet.h"
#include "message-private.h"
#include "server_redundancy.h"
#include "network_server.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "table_skel.h"
#include <pthread.h>
#include "stats.h"

struct u_args
{
    int args;
    struct table_t *tabela; // Substitua 'table_t' pelo tipo real da sua tabela
};

// Informações do socket
int sockfd;                        // Descritor do socket
struct sockaddr_in server, client; // Estruturas para armazenar informações do servidor e do cliente
char received_msg[MAX_MSG + 1];    // String para armazenar a mensagem recebida
int nbytes, count;                 // Variaveis para armazenar o numero de bytes recebidos e o numero de bytes enviados
socklen_t size_client;             // Variavel para armazenar o tamanho da estrutura do cliente

struct statistics_t server_stats; // Estatísticas globais do servidor

extern enum server_status server_status; // Estado do servidor

pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o acesso à tabela

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int network_server_init(short port)
{
    // Criação e verificação do socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro na criação do socket!\n");
        return -1;
    }
    // Preenchemento da estrutura server
    memset(&server, 0, sizeof(struct sockaddr_in)); // Preencher o server a 0s
    server.sin_family = AF_INET;                    // Selecionar familia de endereços
    server.sin_port = htons(port);                  // Selecionar porta convertido para big endian
    server.sin_addr.s_addr = htonl(INADDR_ANY);     // Forçar o servidor a aceitar pedidos de qualquer dos seus endereços IP
    // Configurar opções de socket usando a função setsockopt, permitindo que o socket reutilize endereços de porta e endereços
    int enable = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) < 0)
    {
        printf("Erro a configurar o socket!\n");
        close(sockfd);
        return -1;
    }
    // Associar o socket criado com o endereço e o número da porta especificados na estrutura server
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Erro ao fazer associação do socket à porta!\n");
        close(sockfd);
        return -1;
    }
    // Esperar por conexões
    if (listen(sockfd, 0) < 0)
    {
        perror("Erro na leitura!\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

void *handle_client(void *arg)
{
    if (pthread_detach(pthread_self()) != 0)
    {
        perror("Erro ao fazer detach da thread");
        free(arg);
        return (void *)-1;
    }
    struct u_args *args = arg;
    int connsockfd = args->args;
    struct table_t *table = args->tabela;
    MessageT *msg = NULL;

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.connected_clients++;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    while (1)
    {
        // Receber a mensagem server_table
        struct timeval start, end; // Variaveis para armazenar o tempo de execução da operação

        // Verificar o OPCode da mensagem recebida para saber se a operção é executada localmente ou se é enviada para o servidor primário
        if ((msg->opcode == MESSAGE_T__OPCODE__OP_DEL || msg->opcode == MESSAGE_T__OPCODE__OP_PUT))
        {
            char *backup_address = malloc(DATAMAXLEN);
            if (server_zoo_get_backup(backup_address, DATAMAXLEN) != -1)
            {
                // Establecer conexão com o servidor de backup
                struct rtable_t *rtable = rtable_connect(backup_address);
                MessageT *msg_backup = network_send_receive(rtable, msg);
                if (msg_backup->opcode != MESSAGE_T__OPCODE__OP_ERROR)
                {
                    if (invoke(msg, table) < 0) // FIX NESTA MERDA
                    {
                        perror("Error a processar a resposta na thread");
                        return (void *)-1;
                    }
                }
                else
                {
                    perror("Erro ao enviar mensagem para o servidor de backup");
                    return (void *)-1;
                }
                rtable_disconnect(rtable);
            }
            else
            {
                perror("Erro ao obter o endereço do servidor de backup");
            }

            free(backup_address);
        }
        else
        {
            pthread_mutex_lock(&table_mutex);
            // Iniciar contagem de tempo da operação recorrendo a função gettimeofday
            gettimeofday(&start, NULL);
            // sleep(3); // Sleep utilizado para verificar se o mutex está a funcionar corretamente
            int inv = invoke(msg, table);
            pthread_mutex_unlock(&table_mutex);

            if (inv < 0)
            {
                perror("Error a processar a resposta na thread");
                return (void *)-1;
            }
            else
            {
                // Terminar contagem de tempo da operação recorrendo a função gettimeofday
                gettimeofday(&end, NULL);
                // Incrementar o numero de operações e o total de tempo de execução no statistics_t se a operaçao for bem sucedida e não for um OP_STATS
                if (msg->opcode != MESSAGE_T__OPCODE__OP_STATS)
                {
                    pthread_mutex_lock(&server_stats.stats_mutex);
                    server_stats.total_operations++;
                    server_stats.total_time += (end.tv_sec - start.tv_sec) * 1000000.0;
                    pthread_mutex_unlock(&server_stats.stats_mutex);
                }
                printf("Mensagem enviada!\n");
            }
        }
        // Enviar a resposta ao cliente
        if (network_send(connsockfd, msg) == -1)
        {
            perror("Erro a enviar mensagem ou no invoke!\n");
            // close(connsockfd);
            return (void *)-1;
            // break;
        }

        pthread_mutex_lock(&server_stats.stats_mutex);
        server_stats.connected_clients--;
        pthread_mutex_unlock(&server_stats.stats_mutex);
        close(connsockfd);
        pthread_exit(NULL);
    }
}

/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensahandle_clientgem de-serializada ao skeleton para ser processada
     na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table)
{
    int connsockfd;       // Variavel para armazenar o descritor do socket
    MessageT *msg = NULL; // Variavel mensagem iniciada a NULL para evitar "lixo" na memória

    while (1) // Lidar com a conexão do cliente após esta ser aceite
    {
        connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client);
        if (connsockfd < 0) // Verificação do socket recebido pela função accept
        {
            perror("Error no accept!\n");
            return -1;
        }

        // Criação de uma estrutura para armazenar os argumentos
        struct u_args *argumentos = (struct u_args *)malloc(sizeof(struct u_args));
        if (argumentos == NULL)
        {
            fprintf(stderr, "Erro ao alocar memória para argumentos\n");
            continue;
            // return -1;
        }

        argumentos->args = connsockfd;
        argumentos->tabela = table;
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)argumentos) != 0)
        {
            fprintf(stderr, "Erro ao criar a thread\n");
            continue;
        }
    }
}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket)
{
    short size; // Variavel de apoio para armazenar tmnh da mensagem

    // Receber o tamanho da mensagem
    if (read_all(client_socket, &size, sizeof(short)) <= 0)
    {
        printf("Erro a receber o tamanho da mensagem!\n");
        return NULL;
    }

    // Converter o tamanho de rede de ordem de bytes da network para a ordem de bytes do host
    size = ntohs(size);

    // Verificar se o tamanho da mensagem é válido
    if (size <= 0)
    {
        printf("Tamanho de mensagem inválido!\n");
        return NULL;
    }

    // Alocar memória para o buffer da mensagem
    uint8_t *buf = (uint8_t *)malloc(size);
    if (buf == NULL)
    {
        perror("Erro ao alocar memória para a mensagem recebida");
        return NULL;
    }

    // Receber a mensagem recorrendo ao read_all
    if (read_all(client_socket, buf, size) != size)
    {
        // free(buf);
        perror("Erro ao receber dados do servidor");
        return NULL;
    }

    // Aloca rmemória para a estrutura da mensagem
    // MessageT *msgRecebida = (MessageT *)malloc(sizeof(MessageT));
    // if (msgRecebida == NULL)
    // {
    //     free(buf);
    //     perror("Erro ao alocar memória para a mensagem recebida");
    //     return NULL;
    // }

    // Deserializa a mensagem
    MessageT *msgRecebida = (struct _MessageT *)message_t__unpack(NULL, size, buf);
    free(buf);
    printf("mensagem aqui.... \n");
    printf("Mensagem recebida, key: %s \n", msgRecebida->key);
    // Verifica se a deserialização foi bem-sucedida
    if (msgRecebida == NULL)
    {

        perror("Erro ao deserializar a mensagem");
        return NULL;
    }

    // Retorna a mensagem recebida
    return msgRecebida;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg)
{
    // Serializar a mensagem
    unsigned short len;
    uint8_t *buf = NULL;
    len = message_t__get_packed_size(msg);
    buf = malloc(len);
    if (buf == NULL)
    {
        return -1; // Retorna -1 em caso de erro na alocação de memória
    }

    // Empacotar a mensagem na estrutura de dados
    message_t__pack(msg, buf);

    // Converter o tamanho da mensagem para a ordem de bytes da rede
    int sizeEnviar = htons(len);

    // Enviar o tamanho do buffer
    int sizeEnviado;
    if ((sizeEnviado = write_all(client_socket, &sizeEnviar, sizeof(short)) != sizeof(short)))
    {
        perror("Erro ao enviar tamanho do buffer ao servidor");
        close(client_socket);
        free(buf);
        return -1; // Retorna -1 em caso de erro no envio do tamanho do buffer
    }

    // Libetar a memória ocupada pela mensagem
    message_t__free_unpacked(msg, NULL);
    // free(msg);

    // Enviar o buffer
    int nbytes;
    if ((nbytes = write_all(client_socket, buf, len) != len))
    {
        perror("Erro ao enviar dados ao servidor");
        close(client_socket);
        free(buf);
        return -1; // Retorna -1 em caso de erro no envio do buffer
    }

    // Libetar a memoria do buffer
    free(buf);

    return 0; // Retorna 0 em caso de sucesso
}

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket)
{
    // Fechar o socket
    return close(sockfd);
}
