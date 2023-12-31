#include "table.h"
#include "inet.h"
#include "sdmessage.pb-c.h"
#include "stats.h"

extern struct statistics_t server_stats; // Estatísticas globais do servidor

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists)
{
    // Inicializa a tabela e verifica erros de alocação de memória
    struct table_t *server_table = table_create(n_lists);
    if (server_table == NULL)
    {
        fprintf(stderr, "Erro na inicialização da tabela.\n");
        return NULL;
    }
    return server_table; // Retorna a tabela criada ou NULL em caso de erro
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table)
{
    if (table != NULL)
    {
        table_destroy(table);
        return 0; // Sucesso
    }
    return -1; // Erro
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int invoke(MessageT *msg, struct table_t *table)
{
    printf("Entrei invoke \n");
    if (table == NULL || msg == NULL)
    {
        return -1; // Erro, tabela ou mensagem nulos
    }

    printf("Parametros OK, efetuar operação.... \n");
    // Extrai informações do opcode e conteúdo da mensagem
    short operacao = msg->opcode;
    short conteudo = msg->c_type;

    // Verifica e executa a operação com base no opcode
    if (operacao == MESSAGE_T__OPCODE__OP_SIZE)
    {
        // Operação do tamanho da tabela
        if (table_size(table) == -1)
        {
            // msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            // msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = table_size(table);
        return 0;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_DEL)
    {
        // Operação de remoção de uma entrada da tabela
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        if (conteudo == MESSAGE_T__C_TYPE__CT_KEY)
        {
            if (table_remove(table, msg->key) == -1)
            {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                return -1;
            }
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        return 0;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_GET)
    {
        printf("Entrei no get na função invoke... \n");
        // Operação de encontrar um valor da tabela
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        struct data_t *dataReceived;

        if (conteudo == MESSAGE_T__C_TYPE__CT_KEY)
        {
            printf("Conterudo certo, a efetuar table_get.... \n");
            if ((dataReceived = table_get(table, msg->key)) != NULL)
            {
                msg->value.data = dataReceived->data;
                msg->value.len = dataReceived->datasize;
                // data_destroy(dataReceived);
                return 0;
            }
        }
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_PUT)
    {
        // Operação de inserção ou atualização de um valor na tabela
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

        struct data_t *dataTemp = data_create(msg->value.len, msg->value.data);

        if ((table_put(table, msg->key, dataTemp)) == -1)
        {

            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        return 0;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_GETKEYS)
    {
        // Operação de obtenção de todas as chaves da tabela
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        char **keys;

        if ((keys = table_get_keys(table)) == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        msg->n_keys = table_size(table);
        msg->keys = keys;
        return 0;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_GETTABLE)
    {
        // Operação de obtenção de uma entry com chave key
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    }
    else if (operacao == MESSAGE_T__OPCODE__OP_STATS)
    {
        msg->value.data = malloc(sizeof(struct statistics_t));
        if (msg->value.data == NULL)
        {
            perror("Erro ao alocar memória para a estrutura statistics_t");
            return -1;
        }
        else
        {
            msg->opcode += 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->value.len = sizeof(struct statistics_t);
            memcpy(msg->value.data, &server_stats, sizeof(struct statistics_t));
            return 0;
        }
    }
    return -1;
}