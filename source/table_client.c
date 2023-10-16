/**
 * Rodrigo Barrocas - 53680
 * Matheus Nunes -
 * Pedro Vaz -
 */

#include <signal.h>
#include "network_client.h"
#include "client_stub_private.h"
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
    printf("A iniciar cliente...\n");
    struct rtable_t *rtable; // Definição da variavel de comunicação

    rtable = rtable_connect(argv[1]); // Conectar ao servidor
    if (rtable == 0)                  // Verificar a conexão
    {
        exit(1);
    }

    // Criação de variaveis auxilares
    char comando[MAX_MSG];         // Guarda todo a frase do comando
    const char divisor[3] = " \n"; // Array usado para divisão
    char *operacao;                // Guarda operacao
    char *key;                     // Guarda chave na tabela
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
            }
            else if (strcmp(operacao, "del") == 0) // Comando SIZE
            {
            }
            else if (strcmp(operacao, "get") == 0) // Comando GET
            {
            }
            else if (strcmp(operacao, "put") == 0) // Comando PUT
            {
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