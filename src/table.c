#include <stdlib.h>
#include <string.h>
#include "table-private.h"
#include "table.h"
#include "list.c"

int key_hash(char *key, int l)
{
    if (key == NULL)
        return -1;
    if (l < 1)
        return -1;
    int length = strlen(key);
    int soma = 0;
    int i;

    if (length < 6)
    {
        for (i = 0; i < length; i++)
            soma += key[i];
    }
    else
    {
        for (i = 0; i < 3; i++)
            soma += key[i];
        soma += key[length - 1];
        soma += key[length - 2];
    }
    return soma % l;
}

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n)
{
    // Verificação de parametros
    if (n <= 0)
    {
        return NULL;
    }
    // Criar a tabela alocar na memória o espaço necessário
    struct table_t *table = malloc(sizeof(struct table_t));
    // Verificar o sucesso da alocação de espaço
    if (table == NULL)
        return NULL;
    // Criar o array de listas e reservar o espaço de n estruturas de list_t
    struct list_t **lists = malloc(sizeof(struct list_t) * n);
    // Verificar o sucesso da alocação de espaço
    if (lists == NULL)
        return NULL;

    table->lists = lists; // Colocar o array de listas na table
    int index = 0;        // Variavel para controlo do ciclo
    while (index < n)     // Ciclo para inicialização das listas
    {
        table->lists[index] = list_create(); // Inicializar a lista em index
        index++;
    }
    // Guarda na estrutura table o tamanho da tabela criada
    return table;
}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *table)
{
    // Verificação de parametros
    if (table == NULL)
        return -1;

    // Ciclo para destruir a tablea
    int index = 0;
    while (index < table->size)
    {
        list_destroy(table->lists[index]); // Destruir cada lista(item) da tabela
        index++;
    }
    free(table->lists); // Libertar o espaço alocado para as listas
    free(table);        // Libertar a tabela
    return 0;
}

/* Função para adicionar um par chave-valor à tabela.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na tabela,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *table, char *key, struct data_t *value)
{
    if (table == NULL || key == NULL || value == NULL)
        return -1;

    struct entry_t *entryNova = entry_create(key, value);
    if (entryNova == NULL)
        return -1;
    int numeroEntrada = key_hash(key, table->size);

    if (list_add(table->lists[numeroEntrada], entryNova) == -1)
    {
        entry_destroy(entryNova);
        return -1;
    }
    table->size++;
    printf("aquiii 2 %d", table->size);
    return 0;
}

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou table_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da tabela,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função.
 * Devolve NULL em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
        return NULL;
    struct entry_t *entry;
    int numeroEntrada = key_hash(key, table->size);
    if ((entry = list_get(table->lists[numeroEntrada], key)) == NULL)
    {
        return NULL;
    }
    return data_dup(entry->value);
}

/* Função para remover um elemento da tabela, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação table_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int table_remove(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
        return -1;
    int numeroEntrada = key_hash(key, table->size);

    if (list_remove(table->lists[numeroEntrada], key) == -1)
    {
        return -1;
    }
    table->size--;
    return 0;
}

/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *table)
{
    if (table == NULL)
        return -1;
    printf("%d", table->size);
    return table->size;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **table_get_keys(struct table_t *table)
{
    if (table == NULL)
        return NULL;

    char **listaKeys = malloc((sizeof(char *) * table->size) + 1);
    if (listaKeys == NULL)
    {
        return NULL;
    }
    int index;
    int j = 0, k = 0;
    for (index = 0; index < table->size; index++)
    {
        if (table->lists[index]->head != NULL && table->lists[index]->head->entry != NULL)
        {
            char **temp = list_get_keys(table->lists[index]);
            if (temp == NULL)
                return NULL;
            while (temp[k] != NULL)
            {
                listaKeys[j] = strdup(temp[k]);
                j++;
                k++;
            }
            list_free_keys(temp);
            k = 0;
        }
    }
    if (j == 0)
        return NULL;
    listaKeys[j] = NULL;
    return listaKeys;
}

/* Função que liberta toda a memória alocada por table_get_keys().
 */
int table_free_keys(char **keys)
{
    list_free_keys(keys);
    return 0;
}
