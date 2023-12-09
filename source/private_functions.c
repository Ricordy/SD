/* Grupo 58
 * Rodrigo Barrocas 53680
 * Matheus Nunes 47883
 */

#include "table.h"
#include "table-private.h"
#include "list-private.h"
#include "data.h"
#include "list.h"
#include "entry.h"
#include "private_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash)
 * Em caso de erro retorna NULL.
 */
struct table_t *table_create(int n)
{
    struct table_t *temp = malloc(sizeof(struct table_t));
    temp->n = n;
    temp->lists_ptr = malloc(sizeof(struct list_t *) * n);
    for (int i = 0; i < n; i++)
    {
        temp->lists_ptr[i] = list_create();
    }
    return temp;
}

/* Função para libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table)
{
    for (int i = 0; i < table->n; i++)
    {
        list_destroy(table->lists_ptr[i]);
    }
    free(table->lists_ptr);
    free(table);
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
    if (table == NULL)
    {
        return -1;
    }
    int index = get_hash_index(key, table->n);
    struct list_t *list = table->lists_ptr[index];
    struct entry_t *entry = list_get(list, key);
    if (entry == NULL)
    {
        struct entry_t *const temp = entry_create(strdup(key), data_dup(value));
        list_add(list, temp);
    }
    else
    {
        data_destroy(entry->value);
        entry->value = data_dup(value);
    }
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
    struct entry_t *entry = list_get(table->lists_ptr[get_hash_index(key, table->n)], key);
    return entry == NULL ? NULL : data_dup(entry->value);
}

/* Função para remover um elemento da tabela, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação table_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int table_del(struct table_t *table, char *key)
{
    struct list_t *list = table->lists_ptr[get_hash_index(key, table->n)];
    return list_remove(list, key);
}

/* Função que devolve o número de elementos contidos na tabela.
 */
int table_size(struct table_t *table)
{
    int counter = 0;
    for (int i = 0; i < table->n; i++)
    {
        counter += list_size(table->lists_ptr[i]);
    }
    return counter;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **table_get_keys(struct table_t *table)
{
    char **buffer = malloc(sizeof(char *) * (table_size(table) + 1));
    int index = 0;
    for (int i = 0; i < table->n; i++)
    {
        struct node_t *node = table->lists_ptr[i]->head;
        while (node != NULL)
        {
            buffer[index] = strdup(node->entry->key);
            node = node->next;
            index++;
        }
    }
    buffer[index] = NULL;
    return buffer;
}

/* Função que liberta toda a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys)
{
    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]);
    }
    free(keys);
}

/* Função que imprime o conteúdo da tabela.
 */
void table_print(struct table_t *table)
{
    for (int i = 0; i < table->n; i++)
    {
        printf("Line %d\n", i + 1);
        list_print(table->lists_ptr[i]);
        printf("\n");
    }
}

char *table_to_string(const struct table_t *table)
{
    char *str = list_to_string(table->lists_ptr[0]);
    for (int i = 1; i < table->n; i++)
    {
        char *temp = list_to_string(table->lists_ptr[i]);
        strapp(&str, temp);
        free(temp);
    }
    return str;
}