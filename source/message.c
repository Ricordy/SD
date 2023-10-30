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
            perror("Erro na escrita!"); // Mensagem de erro
            return index;               // Retornar o index do erro para verificar na função chamadora o sucesso da chamada a esta função
        }
        size -= index;
        buffer += index;
    }
    // Retornar o tamanho correto do buffer para verificação na função chamadora o sucesso da chamada a esta função
    return sizeBuffer;
}

// Função que le todas as posições do buffer pelo socket
int read_all(int socket, void *buffer, int size)
{
    int sizeBuffer = size; // Guardar tamanho do buffer a ser retornado
    int index;             // Variavel de apoio
    while (size > 0)       // Ciclo de leitura no socket
    {
        index = read(socket, buffer, size);
        // Verificar o resultado da operação de leitura
        if (index < 0)
        {
            perror("Erro na leitura!"); // Mensagem de erro
            return index;               // Retornar o index do erro para verificar na função chamadora o sucesso da chamada a esta função
        }
        size -= index;
        buffer += index;
    }
    // Retornar o tamanho correto do buffer para verificação na função chamadora o sucesso da chamada a esta função
    return sizeBuffer;
}
