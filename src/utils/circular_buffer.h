#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdlib.h>
//#include <string.h>
#include <stdio.h>
//#include <stddef.h>
#include <stdbool.h>

typedef struct
{
	char *buffer;	 // Buffer de dados (char)
	size_t head;	 // Índice da cabeça (escrita)
	size_t tail;	 // Índice da cauda (leitura)
	size_t size;	 // Quantidade de elementos no buffer
	size_t capacity; // Capacidade total do buffer
} CircularBuffer;

CircularBuffer *circular_buffer_init(size_t capacity);
void circular_buffer_free(CircularBuffer *cb);
bool circular_buffer_write(CircularBuffer *cb, char data);
bool circular_buffer_read(CircularBuffer *cb, char *data);
size_t circular_buffer_write_block(CircularBuffer *cb, const char *data, size_t len);
size_t circular_buffer_read_block(CircularBuffer *cb, char *data, size_t len);
size_t circular_buffer_peek(CircularBuffer *cb, size_t offset, char *data, size_t len) ;
bool circular_buffer_is_full(CircularBuffer *cb);
bool circular_buffer_is_empty(CircularBuffer *cb);
size_t circular_buffer_size(CircularBuffer *cb);
void circular_buffer_clear(CircularBuffer *cb);
void circular_buffer_print(CircularBuffer *cb);

#endif