#include "network_client.h"
#include "inet.h"
#include "client_stub-private.h"
#include "message-private.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) com base na
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable)
{
    // verificar se rtable é NULL
    if (rtable = NULL)
    {
        fprintf(stderr, "rtable vazio.\n");
        return -1;
    }

    // informações do servidor
    rtable->socket.sin_family = AF_INET;                                          // Definiar Address Family (IPV4)
    rtable->socket.sin_port = htons(rtable->server_port);                         // Converter para network byte order (big-endian)
    if (inet_pton(AF_INET, rtable->server_address, &rtable->socket.sin_addr) < 1) // Transformação em binario
    {
        perror("Erro ao converter o endereço IP");
        close(rtable->sockfd);
        return -1;
    }

    // Conectar ao servidor
    if (connect(rtable->sockfd, (struct sockaddr *)&rtable->socket, sizeof(rtable->socket)) < 0) // Verificar a conexão
    {
        close(rtable->sockfd);
        return -1;
    }

    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Tratar de forma apropriada erros de comunicação;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg)
{

    int client_socket = rtable->sockfd;

    // Verificar se socket é válido
    if (client_socket == -1)
    {
        fprintf(stderr, "Erro: Socket não aberto.\n");
        return NULL;
    }

    // Serializar a mensagem
    size_t message_len = message_t__get_packed_size(msg);
    uint8_t *message_buffer = (uint8_t *)malloc(message_len);
    message_t__pack(msg, message_buffer);

    // Tamanho da mensagem
    uint16_t message_size = (uint16_t)message_len;
    if (send(client_socket, &message_size, sizeof(uint16_t), 0) == -1)
    {
        perror("Erro ao enviar mensagem com este tamanho");
        free(message_buffer);
        return NULL;
    }

    // Mensagem serializada
    if (send(client_socket, message_buffer, message_len, 0) == -1)
    {
        perror("Erro ao enviar a mensagem serializada");
        free(message_buffer);
        return NULL;
    }

    // Receção tamanho resposta
    uint16_t response_size;
    if (recv(client_socket, &response_size, sizeof(uint16_t), 0) == -1)
    {
        perror("Erro ao receber o tamanho da resposta");
        free(message_buffer);
        return NULL;
    }

    // Receção resposta
    uint8_t *response_buffer = (uint8_t *)malloc(response_size);
    if (recv(client_socket, response_buffer, response_size, 0) == -1)
    {
        perror("Erro ao receber a resposta");
        free(message_buffer);
        free(response_buffer);
        return NULL;
    }

    // De-serialização da resposta
    MessageT *response = message_t__unpack(NULL, response_size, response_buffer);

    free(message_buffer);
    free(response_buffer);

    return response;
}

/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */

int network_close(struct rtable_t *rtable)
{
    int client_socket = rtable->sockfd;

    // Verificar se socket é válido
    if (client_socket == -1)
    {
        fprintf(stderr, "Erro: Socket não aberto.\n");
        return -1;
    }

    // Fechar o socket
    if (close(client_socket) == -1)
    {
        perror("Erro ao fechar socket cliente");
        return -1;
    }

    // socket -1 = fechado
    rtable->sockfd = -1;

    return 0;
}
