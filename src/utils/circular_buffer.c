#include <stdlib.h>
//#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "circular_buffer.h"

// Inicializa o buffer circular
CircularBuffer* circular_buffer_init(size_t capacity) {
    CircularBuffer* cb = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    if (cb == NULL) return NULL;

    cb->buffer = (char*)malloc(capacity);
    if (cb->buffer == NULL) {
        free(cb);
        return NULL;
    }

    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->size = 0;

    return cb;
}

// Libera a memória do buffer circular
void circular_buffer_free(CircularBuffer* cb) {
    if (cb) {
        free(cb->buffer);
        free(cb);
    }
}

// Adiciona um dado ao buffer
bool circular_buffer_write(CircularBuffer* cb, char data) {
    if (circular_buffer_is_full(cb)) {
        return false;  // Buffer cheio
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->capacity;
    cb->size++;

    return true;
}

// Lê um dado do buffer
bool circular_buffer_read(CircularBuffer* cb, char* data) {
    if (circular_buffer_is_empty(cb)) {
        return false;  // Buffer vazio
    }

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->size--;

    return true;
}
// Função para escrita em bloco (com overwrite automático)
size_t circular_buffer_write_block(CircularBuffer *cb, const char *data, size_t len)
{
    if (!cb || !data || len == 0)
        return 0;

    for (size_t i = 0; i < len; i++)
    {
        if (circular_buffer_is_full(cb))
        {
            cb->tail = (cb->tail + 1) % cb->capacity;
            cb->size--;
        }
        cb->buffer[cb->head] = data[i];
        cb->head = (cb->head + 1) % cb->capacity;
        cb->size++;
    }
    return len;
}

// Função para leitura em bloco
size_t circular_buffer_read_block(CircularBuffer *cb, char *data, size_t len)
{
    if (!cb || !data || len == 0)
        return 0;

    size_t bytes_read = 0;
    while (!circular_buffer_is_empty(cb) && bytes_read < len)
    {
        data[bytes_read++] = cb->buffer[cb->tail];
        cb->tail = (cb->tail + 1) % cb->capacity;
        cb->size--;
    }
    return bytes_read;
}

// Função para peek com offset
size_t circular_buffer_peek(CircularBuffer *cb, size_t offset, char *data, size_t len)
{
    if (!cb || !data || len == 0)
        return 0;

    size_t bytes_peeked = 0;
    size_t pos = (cb->tail + offset) % cb->capacity;

    for (size_t i = 0; i < len; i++)
    {
        if (offset + i >= cb->size)
            break;
        data[i] = cb->buffer[pos];
        pos = (pos + 1) % cb->capacity;
        bytes_peeked++;
    }
    return bytes_peeked;
}
// Verifica se o buffer está cheio
bool circular_buffer_is_full(CircularBuffer* cb) {
    return cb->size == cb->capacity;
}

// Verifica se o buffer está vazio
bool circular_buffer_is_empty(CircularBuffer* cb) {
    return cb->size == 0;
}

// Retorna o tamanho atual do buffer
size_t circular_buffer_size(CircularBuffer* cb) {
    return cb->size;
}

// Limpa o buffer circular
void circular_buffer_clear(CircularBuffer* cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->size = 0;
}

// Exibe o conteúdo do buffer (para debug)
void circular_buffer_print(CircularBuffer* cb) {
    printf("Circular buffer (%zu/%zu bytes): ", cb->size, cb->capacity);
    size_t index = cb->tail;
    for (size_t i = 0; i < cb->size; i++) {
        printf("%c", cb->buffer[index]);
        index = (index + 1) % cb->capacity;
    }
    printf("\n");
}