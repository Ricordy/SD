#include "table.h"
#include "inet.h"
#include "message-private.h"
#include "network_server.h"
#include "table_skel.h"
#include <pthread.h>

struct statistics_t
{
    int total_operations;
    long total_time;
    int connected_clients;
    pthread_mutex_t stats_mutex;
};

int sockfd;
struct sockaddr_in server, client;
char received_msg[MAX_MSG + 1];
int nbytes, count;
socklen_t size_client;

struct statistics_t server_stats; // Estatísticas globais do servidor

pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o acesso à tabela

// Função para preparar um socket de receção de pedidos de ligação num determinado porto.
int network_server_init(short port)
{
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro na criação do socket!\n");
        return -1;
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) < 0)
    {
        printf("Erro a configurar o socket!\n");
        close(sockfd);
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Erro ao fazer associação do socket à porta!\n");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0)
    {
        perror("Erro na leitura!\n");
        close(sockfd);
        return -1;
    }

    // Inicializa as estatísticas e o mutex
    server_stats.total_operations = 0;
    server_stats.total_time = 0;
    server_stats.connected_clients = 0;
    pthread_mutex_init(&server_stats.stats_mutex, NULL);

    return sockfd;
}

void *handle_client(void *arg)
{
    int connsockfd = *((int *)arg);
    MessageT *msg = NULL;

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.connected_clients++;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    while (1)
    {
        msg = network_receive(connsockfd);
        if (msg == NULL)
        {
            close(connsockfd);
            break;
        }

        // Executa a operação enviada pelo cliente
        pthread_mutex_lock(&table_mutex);
        int inv = invoke(msg, table);
        pthread_mutex_unlock(&table_mutex);

        if (inv == -1 || network_send(connsockfd, msg) == -1)
        {
            close(connsockfd);
            break;
        }
    }

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.connected_clients--;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    pthread_exit(NULL);
}

int network_main_loop(int listening_socket, struct table_t *table)
{
    int connsockfd;

    while ((connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client)) > 0)
    {
        printf("Recebeu cliente!\n");
        if (connsockfd == -1)
        {
            perror("Error no accept!\n");
            return -1;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)&connsockfd);
    }

    // Libera recursos após o ciclo
    pthread_mutex_destroy(&server_stats.stats_mutex);

    return 0;
}

MessageT *network_receive(int client_socket)
{
    short size;
    if (read_all(client_socket, &size, sizeof(short)) <= 0)
    {
        printf("Erro a receber!\n");
        return NULL;
    }

    size = ntohs(size);

    if (size <= 0)
    {
        printf("Tamanho de mensagem inválido!\n");
        return NULL;
    }

    uint8_t *buf = (uint8_t *)malloc(size);
    if (buf == NULL)
    {
        perror("Erro ao alocar memória para a mensagem recebida");
        return NULL;
    }

    if (read_all(client_socket, buf, size) != size)
    {
        free(buf);
        perror("Erro ao receber dados do servidor");
        return NULL;
    }

    MessageT *msgRecebida = (struct MessageT *)message_t_unpack(NULL, size, buf);
    free(buf);

    if (msgRecebida == NULL)
    {
        perror("Erro ao deserializar a mensagem");
        return NULL;
    }

    return msgRecebida;
}

int network_send(int client_socket, MessageT *msg)
{
    unsigned short len;
    uint8_t *buf = NULL;
    len = message_t__get_packed_size(msg);
    buf = malloc(len);
    if (buf == NULL)
    {
        return -1;
    }

    message_t__pack(msg, buf);

    int sizeEnviar = htons(len);

    int sizeEnviado;
    if ((sizeEnviado = write_all(client_socket, &sizeEnviar, sizeof(short)) != sizeof(short)))
    {
        perror("Erro ao enviar tamanho do buffer ao servidor");
        close(client_socket);
        free(buf);
        return -1;
    }

    message_t__free_unpacked(msg, NULL);
    free(buf);

    int nbytes;
    if ((nbytes = write_all(client_socket, buf, len) != len))
    {
        perror("Erro ao enviar dados ao servidor");
        close(client_socket);
        free(buf);
        return -1;
    }

    free(buf);

    return 0;
}

int network_server_close(int socket)
{
    return close(sockfd);
}