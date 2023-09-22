// Rodrigo Barrocas 53680
// Pedro Marques XXXXX
// Matheus Nunes XXXXX
// Grupo XX

#ifndef _DATA_H
#define _DATA_H

#include "data.h"
#include <stdlib.h>
#include <string.h>

/* Estrutura que define os dados.
 */
struct data_t
{
    int datasize; /* Tamanho do bloco de dados */
    void *data;   /* Conteúdo arbitrário */
};

/* Função que cria um novo elemento de dados data_t e que inicializa
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_create(int size, void *data)
{
    struct data_t *p;
    if (size <= 0)
        return NULL;
    if (data == NULL)
        return NULL;
    p = (struct data_t *)malloc(sizeof(struct data_t));
    if (p == NULL)
        return NULL;
    p->data = (void *)malloc(size); // Estas 3 linhas deveriam estar aqui tendo em conta
    if (p->data == NULL)            //  a definição da função?
        free(p);                    // MAIS ESTA
    memcpy(p->data, data, size);
    p->datasize = size;

    return p;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
void data_destroy(struct data_t *data)
{
    if (data != NULL)
        if (data->data != NULL)
            free(data->data);

    free(data);
    return 0;
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_dup(struct data_t *data)
{
    if (data == NULL)
        return NULL;
    struct data_t *p = data_create(data->datasize, data->data);
    if (p == NULL)
        return NULL;
    if (data->data == NULL)
        return NULL;
    memcpy(p->data, data->data, data->datasize);
    return p;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_replace(struct data_t *data, int new_size, void *new_data)
{
    if (new_data == NULL)
        return -1;
    if (new_size == NULL)
        return -1;
    if (data == NULL)
        return -1;
    free(data->data);
    data->data = (void *)malloc(new_size);
    memcpy(data->data, new_data, new_size);
    data->datasize = new_size;
    if (data->data == NULL)
        return -1;
    return 0;
}
#endif