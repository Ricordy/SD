// Rodrigo Barrocas 53680
// Pedro Marques XXXXX
// Matheus Nunes 47883
// Grupo XX

#include "data.h"
#include "entry.h"
#include "string.h"
#include "stdlib.h"


/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados de entrada.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_create(char *key, struct data_t *data){
    // Validar args
    if(key == NULL || data == NULL){
        return NULL;
    }

    struct entry_t *newEntry = malloc(sizeof(struct entry_t));
    if(newEntry == NULL){
        return NULL; // Erro a reservar memoria
    }
    newEntry->key = key;
    newEntry->value = data;

    return newEntry;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *entry){
    // Validar args
    if(entry == NULL){
        return -1;
    }

    // Eliminar o data usando o metodo criado
    int result = data_destroy(entry->value);
    // Validar se nao houve erros
    if(result == -1){
        return -1;
    }

    // Libertar memoria do restante
    free(entry->key);
    free(entry);

    return 0;
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_dup(struct entry_t *entry){
    // Validar args
    if(entry == NULL || entry->key == NULL || entry->value == NULL){
        return NULL;
    }

    // Reservar memoria para a struct
    struct entry_t *newEntry = malloc(sizeof(struct entry_t));
    if(newEntry == NULL){
        return NULL; // Erro a reserver memoria
    }

    // Duplicar key
    newEntry->key = malloc(strlen(entry->key)+1);
    if(newEntry->key == NULL){
        return NULL; //Erro a reservar memoria
    }
    strcpy(newEntry->key, entry->key);

    // Duplicar data
    newEntry->value = data_dup(entry->value);
    if(newEntry->value == NULL){
        return NULL; // Erro a duplicar data
    }

    return newEntry;
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    if(entry == NULL || new_key == NULL || new_value == NULL){
        return -1;
    }
    if(entry->key != NULL){
        free(entry->key);
    }
    if(entry->value != NULL){
        int result = data_destroy(entry->value);
        if(result == -1){
            return -1;
        }
    }

    entry->key = new_key;
    entry->value = new_value;

    return 0;
}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se entry1 < entry2,
 * 1 se entry1 > entry2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    // TO-DO
    return NULL;
}
