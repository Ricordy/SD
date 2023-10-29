/**
 * Rodrigo Barrocas - 53680
 * Matheus Nunes -
 * Pedro Vaz -
 */

#include <signal.h>
#include "network_client.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "inet.h"

int main(int argc, char **argv)
{
    // Verificar argumentos
    if (argc != 2 || argc < 0)
    {
        printf("Forma correta de inicar o programa: ./table-client <server>:<port>\n");
        return -1;
    }

    // Iniciar cliente
    struct rtable_t *rtable = rtable_connect(argv[1]); // Definição da variavel de comunicação
    if (rtable == NULL)                                // Verificar a conexão
    {
        exit(1);
    }

    // Criação de variaveis auxilares
    char comando[MAX_MSG];         // Guarda todo a frase do comando
    const char divisor[3] = " \n"; // Array usado para divisão
    char *operacao;                // Guarda operacao
    char *key;                     // Guarda chave da tabela
    char *dados;                   // Guarda dados a modificar

    // Cicle de funcionamento do cliente
    while (1)
    {
        printf("Introduza uma acao: ");
        if (fgets(comando, MAX_MSG, stdin) == NULL) // Verificar se a acao existe
        {
            printf("Nao introduziu nenhum comando!\n");
            break;
        }

        operacao = strtok(comando, divisor);
        // Comando invalido
        if (operacao == 0)
        {
            printf("Introduza um comando valido!\n");
            printf("Lista de comandos validos:\n");
            printf(" size\n");
            printf(" del <key>\n");
            printf(" get <key>\n");
            printf(" put <key> <data>\n");
            printf(" getkeys\n");
            printf(" quit\n");
        }
        // Comando valido
        else
        {
            if (strcmp(operacao, "quit") == 0) // Comando QUIT
            {
                printf("A terminar a ligacao...\n");
                break;
            }
            else if ((strcmp(operacao, "size") == 0)) // Comando SIZE
            {
                printf("A obter o tamanho da tabela...  %d\n", rtable->sockfd);
                int tamanho = rtable_size(rtable);
                printf("Tamanho -> %d\n", tamanho);
                printf("Tamanho -> %d\n", tamanho);
            }
            else if (strcmp(operacao, "del") == 0) // Comando DEL
            {
                key = strtok(NULL, divisor);
                if (key == NULL) // Verificar se foi passada uma key no comando
                {
                    printf("Este comando necessita de mais um parametro: del <key>");
                }
                else // Caso em que a key foi passada
                {
                    if (rtable_del(rtable, key) == -1) // Eliminar e verificar o resultado da operação
                    {
                        printf("Erro a apagar, verifique a existencia da <key> na tabela\n"); // Mensagem de erro
                    }
                    else
                    {
                        printf("Eliminado com sucesso!\n"); // Mensagem de sucesso
                    }
                }
            }
            else if (strcmp(operacao, "get") == 0) // Comando GET
            {
                key = strtok(NULL, divisor);
                if (key == NULL) // Verificar se foi passada uma key no comando
                {
                    printf("Este comando necessita de mais um parametro: get <key>");
                }
                else
                {
                    struct data_t *dadosRecebidos;                          // Variavel de apoio para guardar dados/erro
                    if ((dadosRecebidos = rtable_get(rtable, key)) == NULL) // Verificação da existencia dos dados na tabela e atribuição do valor à variavel
                    {
                        printf("Erro a obter o valor, key invalida!\n"); // Mensagem de erro
                    }
                    else
                    {
                        printf("Dados obtidos com sucesso\n");
                        printf("Dados: %s \n", (char *)dadosRecebidos->data);
                        data_destroy(dadosRecebidos); // Destruir variavel auxiliar
                    }
                }
            }
            else if (strcmp(operacao, "put") == 0) // Comando PUT
            {
                key = strtok(NULL, divisor); // Obter o valor da key
                dados = strtok(NULL, "\n");  // Obter o valor dos dados
                if (dados == NULL)
                {
                    printf("O comando necessita de mais um parametro, put <key> <data>\n");
                }
                else
                {
                    struct data_t *data = data_create(strlen(dados) + 1, dados);
                    struct entry_t *entry = entry_create(key, data);
                    if (rtable_put(rtable, entry) == -1)
                    {
                        printf("Erro a colocar os dados na tabela!");
                    }
                    else
                    {
                        printf("Dados colocados com sucesso!");
                    }
                    free(data);
                    free(entry);
                }
            }
            else if (strcmp(operacao, "getkeys") == 0) // Comando GETKEYS
            {
                char **keys = rtable_get_keys(rtable); // Criação de variavel de apoio
                if (keys == NULL)                      // Verificação dos dados obtidos na variavel de apoio
                {
                    printf("Erro a obter as keys!\n"); // Mensagem de erro
                }
                else
                {
                    int index;                                           // Variavel de apoio
                    int numeroEntradas = rtable_size(rtable);            // Variavel de apoio com tamanho da tabela
                    for (index = 0; index != numeroEntradas; index += 1) // Ciclio para imprimir as keys na tela
                    {
                        printf("%s \n", keys[index]);
                    }
                    // Libertar a memoria alocada pela variavel de apoio
                    rtable_free_keys(keys);
                }
            }
            else // Comando intruduzido é invalido
            {
                printf("Introduza um comando valido!\n");
                printf("Lista de comandos validos:\n");
                printf(" size\n");
                printf(" del <key>\n");
                printf(" get <key>\n");
                printf(" put <key> <data>\n");
                printf(" getkeys\n");
                printf(" quit\n");
            }
        }
    }
}