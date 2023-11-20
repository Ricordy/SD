#include "client_stub-private.h"
#include "network_client.h"
#include "inet.h"
#include "message-private.h"

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port)
{
    // Criação e verificação da variavel que conterá a estrutura
    struct rtable_t *const tcp_table = malloc(sizeof(struct rtable_t));
    if (tcp_table == NULL)
    {
        return NULL; // Retornar erro
    }
    tcp_table->server_address = NULL;
    tcp_table->server_port = -1;

    for (int i = 0; i < strlen(address_port); i++) // Percorrer o array de char fornecido
    {
        if (address_port[i] == ':') // quando encontramos a divisão ip:porto
        {
            tcp_table->server_port = atoi(address_port + i + 1);                     // Converter a string relativa ao porto
            tcp_table->server_address = malloc(sizeof(char) * (i + 1));              // Reservar o espaço necessário para o ip do servidor
            memcpy(tcp_table->server_address, address_port, sizeof(char) * (i + 1)); // copiar o valor do ip para a o endereço da memoria que definimos anteriormente
            ((char *)tcp_table->server_address)[i] = '\0';                           // Colocar o caracter de fim de string
        }
    }
    tcp_table->sockfd = socket(AF_INET, SOCK_STREAM, 0);                                              // Criação do socket para comunicação
    if (tcp_table->sockfd == -1 || tcp_table->server_address == NULL || tcp_table->server_port == -1) // Verificação da criação da estrutura
    {
        return NULL; // Retornar erro
    }
    printf("A conectar ao servidor no socket: %d\n", tcp_table->sockfd);
    int networkstatus = network_connect(tcp_table);
    if (networkstatus == -1) // Tentar conectar ao servidor
    {
        return NULL; // Retornar erro
    }
    else
    {
        return tcp_table; // Retornar estrutura conectada ao servidor
    }
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem, ou -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable)
{
    int resposta = network_close(rtable); // Tentar terminar a conexão com o servidor
    free(rtable);                         // Libertar a memória da estrutura
    return resposta;                      // Retornar a resposta da função network_close() que dirá se terminou ou não a conexão
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Retorna 0 (OK, em adição/substituição), ou -1 (erro).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry)
{
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket

    if (mensagem == NULL) // Verificação da alocação de espaço
    {
        return -1;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return -1;
    }

    message_t__init(mensagemConvert);                           // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                     // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_PUT;   // Colocar o opcode da operção
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_ENTRY; // Colocar o tipo de dados

    // -------------------------------- VERIFICAR ESTAS LINHAS------------------------------------------------
    // mensagem->msgConvert->entry = entry;                      // colocar os dados na mensagem
    mensagem->msgConvert->key = entry->key;                   // Colocar a chave da entrada
    mensagem->msgConvert->value.len = entry->value->datasize; // Colocar o tamanho da data
    mensagem->msgConvert->value.data = entry->value->data;    // Colocar a data
    // -------------------------------------------------------------------------------------------------------

    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert); // Escrever mensagem no socket
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)           // Verificar operação de escrita no socket
    {
        printf("Erro na inserção da entrada!\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL); // Libertar o espaço ocupado pela mensagem
        free(mensagem);                                       // Libertar variavel de comunicação
        return -1;
    }
    else
    {
        free(mensagemConvert); // Libertar o espaço ocupado pela mensagem
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagem); // Libertar variavel de comunicação

        return 0;
    }
}

/* Retorna o elemento da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key)
{
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket
    if (mensagem == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }

    message_t__init(mensagemConvert);                         // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                   // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_GET; // Colocar o opcode da operção
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_KEY; // Colocar o tipo de dados enviados
    mensagem->msgConvert->key = key;                          // Colocar a key recebida nos parametros
    if (mensagem->msgConvert->key == NULL)                    // Verificar se houve algum erro na colocação da key recebida na variavel de comunicação
    {
        free(mensagem->msgConvert->key); // Libertar o espaço reservado
        return NULL;
    }
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert); // Enivar o pedido ao servidor
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)           // Verificar se encontrou alguma key igual à pedida
    {
        printf("Erro a obter elemento da tabela\n");
        // Libertar o espaço reservado
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagem);
        // Retorno de não encontrar
        return NULL;
    }

    struct data_t *dataConvert = data_create(mensagem->msgConvert->value.len, mensagem->msgConvert->value.data); // Ciração de uma variavel que retenha o valor da data associada à key
    mensagem->msgConvert->value.data = NULL;                                                                     // Colocar o valor da data a null
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    return dataConvert;
}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key)
{
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket

    if (mensagem == NULL) // Verificação da alocação de espaço
    {
        return -1;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return -1;
    }
    message_t__init(mensagemConvert);                         // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                   // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_DEL; // Colocar o opcode da operção
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_KEY; // Colocar o tipo de dados enviados
    mensagem->msgConvert->key = key;                          // Colocar a chave da entrada recebida
    if (mensagem->msgConvert->key == NULL)                    // Verificar se houve algum erro na colocação da key recebida na variavel de comunicação
    {
        return -1;
    }
    // Enviar o pedido para o servidor e processar a resposta
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert);
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        // Imprimir erro e libertar memoria reservada
        printf("Erro a eliminar elemento da tabela\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagem);
        return -1;
    }
    // Libertar memoria reservada e retornar codigo de sucesso
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    return 0;
}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable)
{
    printf("A obter o tamanho da tabela... no rtbale_size %d\n", rtable->sockfd);
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket

    if (mensagem == NULL) // Verificação da alocação de espaço
    {
        printf("Erro a obter numero de elementos da tabela\n");
        return -1;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        printf("Erro a obter numero de elementos da tabela\n");
        return -1;
    }
    printf("A inicar a mensagem\n");
    message_t__init(mensagemConvert);                          // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                    // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_SIZE; // Colocar o opcode da operação
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_NONE; // Colocar o tipo de dados enviados no caso nenhum
    printf("A enviar a mensagem para o network_send_recieve em table_size\n");
    // Enviar o pedido para o servidor e processar a resposta
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert);
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        // Imprimir erro e libertar memoria reservada
        printf("Erro a obter numero de elementos da tabela\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagem);
        return -1;
    }
    printf("A libertar a mensagem\n");
    // Libertar memoria reservada e retornar tamanho pedido
    int tamanho = mensagem->msgConvert->n_keys;
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    printf("A retornar o tamanho\n");
    return tamanho;
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable)
{
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket
    if (mensagem == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }
    message_t__init(mensagemConvert);                             // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                       // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_GETKEYS; // Colocar o opcode da operação
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_NONE;    // Colocar o tipo de dados enviados no caso nenhum

    // Enviar o pedido para o servidor e processar a resposta
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert);
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        // Imprimir erro e libertar memoria reservada
        printf("Erro a obter keys da tabela\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagemConvert);
        free(mensagem);
        return NULL;
    }
    // Libertar memoria reservada e retornar as keys pedidas
    char **keys = mensagem->msgConvert->keys;
    mensagem->msgConvert->keys = NULL;
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    return keys;
}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys)
{
    int index = 0;              // Variavel de apoio para correr o array
    while (keys[index] != NULL) // Correr o array em ciclo para libertar a memoria de cada key
    {
        free(keys[index]); // Libertação da memória
        index += 1;
    }
    free(keys); // Libertação do espaço reservado pela variavel que continha cada todas keys
}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable)
{
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket
    if (mensagem == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }
    message_t__init(mensagemConvert);                              // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                        // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_GETTABLE; // Colocar o opcode da operação
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_NONE;     // Colocar o tipo de dados enviados no caso nenhum

    // Enviar o pedido para o servidor e processar a resposta
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert);
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        // Imprimir erro e libertar memoria reservada
        printf("Erro a obter a tabela\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagemConvert);
        free(mensagem);
        return NULL;
    }
    // Libertar memoria reservada e retornar as keys pedidas
    struct entry_t **entries = (struct entry_t **)mensagem->msgConvert->entries;
    mensagem->msgConvert->entries = NULL;
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    return entries;
}

/* Função que devolvce as estatisticas da tabela.
 */

struct statistics_t *rtable_stats(struct rtable_t *rtable)
{
    printf("Bonmito bonito");
    struct message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket
    if (mensagem == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }

    struct _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio
    if (mensagemConvert == NULL)                                          // Verificação da alocação de espaço
    {
        return NULL;
    }
    message_t__init(mensagemConvert);                           // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                     // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_STATS; // Colocar o opcode da operação
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_NONE;  // Colocar o tipo de dados enviados no caso nenhum

    // Enviar o pedido para o servidor e processar a resposta
    mensagem->msgConvert = network_send_receive(rtable, mensagem->msgConvert);
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        // Imprimir erro e libertar memoria reservada
        printf("Erro a obter as estatisticas da tabela\n");
        message_t__free_unpacked(mensagem->msgConvert, NULL);
        free(mensagemConvert);
        free(mensagem);
        return NULL;
    }
    // Libertar memoria reservada e retornar as keys pedidas
    struct statistics_t *stats = (struct statistics_t *)mensagem->msgConvert->value.data;
    mensagem->msgConvert->value.data = NULL;
    mensagem->msgConvert->value.len = 0;
    message_t__free_unpacked(mensagem->msgConvert, NULL);
    free(mensagemConvert);
    free(mensagem);
    return stats;
}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries)
{
    int index = 0;                 // Variavel de apoio para correr o array
    while (entries[index] != NULL) // Correr o array em ciclo para destruir cada entry
    {
        entry_destroy(entries[index]); // Destruição da entrada
        index += 1;
    }
    free(entries); // Libertação do espaço reservado pela variavel que continha cada todas entries
}