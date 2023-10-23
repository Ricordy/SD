#include "client_stub-private.h"
#include "network_client.h"
#include "inet.h"
#include "message-private.h"
#include "protobuf-c/sdmessage.pb-c.h"

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
    // Preencher a estrutura com os dados passados nos argumentos
    tcp_table->server_address = strtok((char *)address_port, ":");
    tcp_table->server_port = htons(atoi(strtok(NULL, "")));
    tcp_table->sockfd = socket(AF_INET, SOCK_STREAM, 0); // Criação do socket para comunicação

    if (tcp_table->sockfd == -1 || tcp_table->server_address == NULL || tcp_table->server_port == -1) // Verificação da criação da estrutura
    {
        return NULL; // Retornar erro
    }

    if (network_connect(tcp_table) == -1) // Tentar conectar ao servidor
    {
        return NULL; // Retornar erro
    }
    else
    {
        return tcp_table // Retornar estrutura conectada ao servidor
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
    message_t *mensagem = malloc(sizeof(struct message_t)); // Criação da variavel para escrita da mensagem no socket
    if (mensagem == NULL)                                   // Verificação da alocação de espaço
    {
        return -1;
    }

    _MessageT *mensagemConvert = malloc(sizeof(struct _MessageT)); // Criação da variavel de apoio

    message_t__init(mensagemConvert);                                // Inicialização da variavel de apoio
    mensagem->msgConvert = mensagemConvert;                          // Colocar a variavel de apoio na variavel de comunicação com o socket
    mensagem->msgConvert->opcode = MESSAGE_T__OPCODE__OP_PUT;        // Colocar o opcode da operção
    mensagem->msgConvert->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;      // Colocar o tipo de dados
    mensagem->msgConvert->entry = entry;                             // colocar os dados na mensagem
    mensagem->msgConvert->key = entry->key;                          // Colocar a chave da entrada
    mensagem = network_send_receive(rtable, mensagem);               // Escrever mensagem no socket
    if (mensagem->msgConvert->opcode == MESSAGE_T__OPCODE__OP_ERROR) // Verificar operação de escrita no socket
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
struct data_t *rtable_get(struct rtable_t *rtable, char *key) {}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key) {}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable) {}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable) {}

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
struct entry_t **rtable_get_table(struct rtable_t *rtable) {}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries)
{
    int index = 0;                 // Variavel de apoio para correr o array
    while (entries[index] != NULL) // Correr o array em ciclo para destruir cada entry
    {
        entry_destroy(keys[index]); // Destruição da entrada
        index += 1;
    }
    free(entries); // Libertação do espaço reservado pela variavel que continha cada todas entries
}