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

    
}