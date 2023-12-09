#include <signal.h>
#include "inet.h"
#include "table_skel.h"
#include "server_redundancy.h"
#include "network_server.h"
#include "stats.h"

struct table_t *server_table;     // Tabela de dados
enum server_status server_status; // Estado do servidor

// Manipulador para o sinal SIGINT (Ctrl+C)
void termination_handler(int signum)
{
    fprintf(stderr, "\nEncerrando o servidor.\n");
    network_server_close(signum);     // Fecha o servidor de rede
    table_skel_destroy(server_table); // Destrói a tabela
    server_zoo_close();
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <porta> <número_de_listas>\n", argv[0]);
        return 1; // Saída de erro indicando uso incorreto
    }
    if (server_zoo_init(argv[3]) == -1)
    {
        fprintf(stderr, "Erro a iniciar Zookeeper");
        return 1;
    }

    signal(SIGINT, termination_handler); // Registra o manipulador para SIGINT

    char *address;
    myIp(&address);

    char *heap = malloc((strlen(address) + 1) * sizeof(char));
    memcpy(heap, address, (strlen(address) + 1) * sizeof(char));

    strappend(&heap, ":");
    strappend(&heap, argv[1]);

    do
    {
        server_status = server_zoo_register(heap, strlen(heap) + 1);
        if (server_status == REPEAT)
        {
            sleep(1);
        }
    } while (server_status == REPEAT);

    free(heap);
    server_zoo_setwatch(&server_status);

    if (server_status != NONE && server_status != ERROR)
    {

        int porta = atoi(argv[1]);
        int numero_de_listas = atoi(argv[2]);
        // Inicializa as tabelas
        server_table = table_skel_init(numero_de_listas);
        if (server_table == NULL)
        {
            fprintf(stderr, "Erro ao criar as tabelas!\n");
            return 1; // Saída de erro indicando falha ao criar as tabelas
        }

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

        // Inicia o loop principal de rede
        int status = network_main_loop(socket, server_table);
        if (status < 0)
        {
            perror("Erro a preparar o servidor");
        }
    }
}
