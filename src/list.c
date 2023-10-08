#include "data.h"
#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "string.h"
#include "stdlib.h"

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create()
{
    struct list_t *newList = malloc(sizeof(struct list_t));
    if (newList == NULL)
    {
        return NULL; // Erro a reservar memoria
    }
    newList->size = 0;
    newList->head = NULL;
    return newList;
}

/* Função auxiliar que elimina um nó da lista e chama a si mesma para eliminar o próximo nó */
void recursive_destroy(struct node_t *node)
{
    if (node == NULL)
        return;

    recursive_destroy(node->next); // Destruir recursivamente o nó seguinte
    entry_destroy(node->entry);    // Destroi a entrada na
    free(node);                    // Libertar o espaço em memória do node
}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_destroy(struct list_t *list)
{
    if (list == NULL)
        return -1;

    recursive_destroy(list->head); // Recursivamente destruir cada nó
    free(list);                    // Libertar o espaço em memória da lista
    return 0;
}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *list, struct entry_t *entry)
{
    // Verificar parametros
    if (list == NULL || entry == NULL)
        return -1;

    // Criar esturtura para o novo nó
    struct node_t *newNode = malloc(sizeof(struct node_t));
    // Verificar se o espaço foi alocado corretamente
    if (newNode == NULL)
        return -1;

    // Passar a informação para
    newNode->entry = entry;
    newNode->next = NULL;

    // Se a lsta estiver vazia o novo nó e a head
    if (list->head == NULL)
    {
        list->head = newNode;
        list->size++;
        return 0;
    }

    // Verificar os nós da lista para colocar o novo nó
    struct node_t *current = list->head;
    struct node_t *previous = NULL;

    // Correr a lista até ao fim ou encontrar a posição
    while (current != NULL)
    {
        int cmp = entry_compare(entry, current->entry);
        if (cmp == 0) // Se o resultado é zero, as keys são iguais e deve-se substituir pela nova entry
        {
            entry_destroy(current->entry); // Destruir a entrada já existente
            current->entry = entry;        // Colocar a nova entry no lugar da entrada antiga
            free(newNode);                 // Libertar o espaço utilizado pelo novo nó.
            return 1;
        }
        else if (cmp == -1) // Entry1 é menor
        {
            if (previous == NULL)         // Se ainda não foi encontrado um nó menor
                list->head = newNode;     // Colocar o novo nó na head
            else                          // Existind um nó maior
                previous->next = newNode; // colocar no nó anterior a informação deste no

            newNode->next = current; // Colocar no novo nó a informação de quem é o proximo nó
            list->size++;            // Aumentar o tamanho da lista
            return 0;
        }
        else // Continuar a procura
        {
            previous = current; // Alterar as variavel de ajuda
            current = current->next;
        }
    }
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *list, char *key){
    if(list == NULL || key == NULL){
        return -1;
    }
    struct node_t *current = list->head;

    // Se a key for a head da list
    if(strcmp(key, current->entry->key) == 0){
        entry_destroy(current->entry);
        list->head = current->next;
        free(current);
        list->size--;
        return 0;
    }
    // Se a key estiver na tail da list
    while (current != NULL){
        struct node_t *next = current->next;
        if(strcmp(key, next->entry->key) == 0){
            entry_destroy(next->entry);
            current->next = next->next;
            free(next);
            list->size--;
            return 0;
        }
        current = current->next;
    }
    
    return -1;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
 */
struct entry_t *list_get(struct list_t *list, char *key)
{
    return NULL;
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *list)
{
    return NULL;
}

/* Função que constrói um array de char* com a cópia de todas as keys na
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *list)
{
    return NULL;
}

/* Função que liberta a memória ocupada pelo array de keys obtido pela
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys)
{
    return NULL;
}
