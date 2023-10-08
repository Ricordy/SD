// Rodrigo Barrocas 53680
// Pedro Vaz 46368
// Matheus Nunes 47883
// Grupo 58

#include "data.h"
#include <stdlib.h>
#include <string.h>


/* Função que cria um novo elemento de dados data_t e que inicializa
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_create(int size, void *data){
  
    // Validar args
    if(data == NULL || size <= 0){
        return NULL;
    }

    struct data_t *newData = malloc(sizeof(struct data_t));
    if(newData == NULL){
        return NULL; // Falha na reserva de memoria
    }
    newData->data = data;
    newData->datasize = size;
    return newData;
    


    
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_destroy(struct data_t *data){

    // Validar args
    if(data == NULL || data->data == NULL){
        return -1;
    }
    free(data->data);
    free(data);
    return 0;
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_dup(struct data_t *data){

    // Validar args
    if(data == NULL || data->data == NULL || data->datasize <=0){
        return NULL;
    }
    struct data_t *newData = malloc(sizeof(struct data_t));
    if(newData == NULL){
        return NULL; // Falha na reserva de memoria
    }
    newData->data = malloc(data->datasize);
    if(newData->data == NULL){
        return NULL; // Falha na reserva de memoria
    }
    memcpy(newData->data, data->data, data->datasize);
    newData->datasize = data->datasize;
    return newData;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_replace(struct data_t *data, int new_size, void *new_data){
    // Validar args
    if(data == NULL || new_size <= 0 || new_data == NULL){
        return -1; 
    }
    if(data->data != NULL){
        free(data->data);
    }

    data->data = malloc(new_size);
    if(data->data == NULL){
        return -1; // Falha na reserva de memoria
    }
    memcpy(data->data, new_data, new_size);
    data->datasize = new_size;
    return 0;

}
