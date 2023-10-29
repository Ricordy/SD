#include "message-private.h"
#include "inet.h"

// Função que envia todas as posições do buffer pelo socket
int write_all(int socket, void *buffer, int size)
{

    int sizeBuffer = size; // Guardar tamanho do buffer a ser retornado
    int index;             // Variavel de apoio
    while (size > 0)       // Ciclo de escrita no socket
    {
        // Verificar o resultado da operação de escrita
        if ((index = write(socket, buffer, size)) < 0)
        {
            printf("Write all failed: index: %d\n", index);
            perror("Erro na escrita!"); // Mensagem de erro
            return index;               // Retornar o index do erro para verificar na função chamadora o sucesso da chamada a esta função
        }
        printf("Write all: index: %d\n", index);
        size -= index;
        buffer += index;
    }
    // Retornar o tamanho correto do buffer para verificação na função chamadora o sucesso da chamada a esta função
    return sizeBuffer;
}

// Função que le todas as posições do buffer pelo socket
int read_all(int socket, void *buffer, int size)
{
    printf("Read all: socket: %d     %d\n ", socket, size);

    int sizeBuffer = size; // Guardar tamanho do buffer a ser retornado
    int index;             // Variavel de apoio
    while (size > 0)       // Ciclo de leitura no socket
    {
        printf("Read all: size: %d\n", size);
        printf("Read all error: size: %ld\n", read(socket, buffer, sizeBuffer));
        // Verificar o resultado da operação de leitura
        if ((index = read(socket, buffer, size)) < 0)
        {
            printf("Read all error: size: %d\n", index);
            perror("Erro na leitura!"); // Mensagem de erro
            return index;               // Retornar o index do erro para verificar na função chamadora o sucesso da chamada a esta função
        }
        size -= index;
        buffer += index;
    }
    // Retornar o tamanho correto do buffer para verificação na função chamadora o sucesso da chamada a esta função
    return sizeBuffer;
}
