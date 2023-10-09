#include "data.h"
#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

void list_print(struct list_t *list)
{
    printf("tamanho da lista: \n %i", list->size);
    struct node_t *current = list->head;
    while (current != NULL)
    {
        printf("########################## \n");
        printf("entry: %s valueSize: %d \n", current->entry->key, current->entry->value->datasize);
        printf("########################## \n");
        current = current->next;
    }
    return;
}

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
    {
        return -1;
    }

    // Criar esturtura para o novo nó
    struct node_t *newNode = malloc(sizeof(struct node_t));
    // Verificar se o espaço foi alocado corretamente
    if (newNode == NULL)
    {
        return -1;
    }

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
    // Caso não tenha sido encontrado durante o ciclo o seu lugar na lista
    previous->next = newNode; // Colocar no final da lista
    list->size++;             // Aumentar o tamanho da lista
    return 0;
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *list, char *key)
{
    // Verificação de parametros
    if (list == NULL || key == NULL || list->head == NULL)
    {
        return -1;
    }

    // Criação de variaveis de apoio
    struct node_t *current = list->head;
    struct node_t *previous = NULL;

    while (current != NULL)
    {
        if (strcmp(key, current->entry->key) == 0)
        {
            if (previous == NULL)
            {
                // Key is in the head of the list
                list->head = current->next;
            }
            else
            {
                // Key is in the middle or tail of the list
                previous->next = current->next;
            }

            entry_destroy(current->entry);
            free(current);
            list->size--;
            return 0; // Key found and removed
        }

        previous = current;
        current = current->next;
    }

    return 1; // Key not found in the list
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
 */
struct entry_t *list_get(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL || list->head == NULL)
    {
        return NULL;
    }

    struct node_t *current = list->head;
    while (current != NULL)
    {
        if (strcmp(key, current->entry->key) == 0)
        {
            return current->entry;
        }
        current = current->next;
    }

    // Caso não exista
    // printf("nao existe");
    return NULL;
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }

    return list->size;
}

/* Função que constrói um array de char* com a cópia de todas as keys na
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *list)
{
    // Verificação de parametros
    if (list == NULL)
    {
        return NULL;
    }

    // Alocar espaço necessário para o array de keys
    char **keys = malloc((list->size + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL; // Erro na alocação de memória
    }

    // Criação de uma variavel de apoio com o valor do nó atual
    struct node_t *current = list->head; // Valor inicial - cabeça da list
    int counter = 0;                     // Variavel de contagem
    while (current != NULL)              // Ciclo para correr cada nó
    {
        keys[counter] = strdup(current->entry->key); // Colocar a key no novo array
        current = current->next;                     // Atualizar a variavel de apoio
        counter++;                                   // Aumentar o contador
    }

    keys[counter] = NULL; // Colocar o valor NULL no ultimo elemento do array
    return keys;          // retorno do novo array de chaves
}

/* Função que liberta a memória ocupada pelo array de keys obtido pela
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys)
{
    // Verificação de parametros
    if (keys == NULL)
        return -1;

    int counter = 0;              // Variavel de controlo do ciclo
    while (keys[counter] != NULL) // Ciclo para libertar cada posição do array da memória
    {
        free(keys[counter]); // Libertar a posição atual
        counter++;           // Mover o contador para a proxima posição
    }
    free(keys); // Libertar as chaves
    return 0;
}
