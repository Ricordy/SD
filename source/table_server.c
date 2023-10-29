#include <signal.h>
#include "inet.h"
#include "table_skel.h"
#include "network_server.h"

// Manipulador para o sinal SIGINT (Ctrl+C)
void termination_handler(int signum)
{
    fprintf(stderr, "\nEncerrando o servidor.\n");
    network_server_close(signum); // Fecha o servidor de rede
    table_skel_destroy();         // Destrói a tabela
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <porta> <número_de_listas>\n", argv[0]);
        return 1; // Saída de erro indicando uso incorreto
    }

    signal(SIGINT, termination_handler); // Registra o manipulador para SIGINT

    int porta = atoi(argv[1]);
    int numero_de_listas = atoi(argv[2]);

    if (porta <= 0 || numero_de_listas <= 0)
    {
        fprintf(stderr, "A porta e o número de listas devem ser números positivos.\n");
        return 1; // Saída de erro indicando argumentos inválidos
    }

    // Inicializa o servidor de rede
    int socket = network_server_init((short)porta);
    if (socket == -1)
    {
        fprintf(stderr, "Erro ao criar o socket!\n");
        return 1; // Saída de erro indicando falha ao criar o socket
    }

    // Inicializa as tabelas
    int result = table_skel_init(numero_de_listas);
    if (result == -1)
    {
        fprintf(stderr, "Erro ao criar as tabelas!\n");
        return 1; // Saída de erro indicando falha ao criar as tabelas
    }

    // Inicia o loop principal de rede
    result = network_main_loop(socket);

    // Fecha o servidor de rede e destrói as tabelas
    network_server_close(socket);
    table_skel_destroy();

    return result;
}
