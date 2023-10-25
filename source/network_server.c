#include "table.h"
#include "inet.h"
#include "sdmessage.pb-c.h"

// Informações do socket
int sockfd;
struct sockaddr_in server, client;
char str[MAX_MSG + 1];
int nbytes, count;
socklen_t size_client;

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
    if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&enable, sizeof(enable)) < 0)
    {
        printf("Erro a configurar o socket!\n");
        close(sockfd);
        exit(2);
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

/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada
     na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table);

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket);

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg);

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket);
