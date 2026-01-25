#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
struct ring_buffer
{
    uint8_t *buffer;
    uint8_t head, tail, size, elem_size;
    bool full;
};
/*Creating more space for the byte buffer by doing size*sizeof(type)*/
#define RING_BUFFER(name, buff_size, type, storage)                                                \
    static_assert(buff_size < UINT8_MAX);                                                          \
    storage uint8_t buffer[buff_size * sizeof(type)] = { 0 };                                      \
    storage struct ring_buffer name = { .size = buff_size,                                         \
                                        .elem_size = sizeof(type),                                 \
                                        .buffer = buffer };

#define LOCAL_RING_BUFFER(name, buff_size, type) RING_BUFFER(name, buff_size, type)
#define STATIC_RING_BUFFER(name, buff_size, type) RING_BUFFER(name, buff_size, type, static)

// void* as data can be any type
void ring_buffer_put(struct ring_buffer *rb, const void *data);
void ring_buffer_get(struct ring_buffer *rb, void *data);

void ring_buffer_peek_tail(const struct ring_buffer *rb, void *data);
void ring_buffer_peek_head(const struct ring_buffer *rb, void *data, uint8_t offset);

uint8_t ring_buffer_count(const struct ring_buffer *rb);
bool ring_buffer_full(const struct ring_buffer *rb);
bool ring_buffer_empty(const struct ring_buffer *rb);
#endif // RING_BUFFER_H