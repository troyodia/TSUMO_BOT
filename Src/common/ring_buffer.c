#include "ring_buffer.h"
#include "assert_handler.h"
#include <string.h>

void ring_buffer_put(struct ring_buffer *rb, const void *data)
{
    if (rb->full) {
        ring_buffer_get(rb, NULL);
    }
    // multiply by elem_size as the size of the element is unkown
    memcpy(&rb->buffer[rb->head * rb->elem_size], data, rb->elem_size);
    rb->head++;
    // Didnt use modulu for space savings
    if (rb->head == rb->size) {
        rb->head = 0;
    }
    if (rb->head == rb->tail) {
        rb->full = true;
    }
}
void ring_buffer_get(struct ring_buffer *rb, void *data)
{
    ASSERT(!ring_buffer_empty(rb));
    if (data) {
        memcpy(data, &rb->buffer[rb->tail * rb->elem_size], rb->elem_size);
    }
    rb->tail++;
    if (rb->tail == rb->size) {
        rb->tail = 0;
    }
    if (rb->full) {
        rb->full = false;
    }
}

void ring_buffer_peek_tail(const struct ring_buffer *rb, void *data)
{
    ASSERT(!ring_buffer_empty(rb));
    if (data) {
        memcpy(data, &rb->buffer[rb->tail * rb->elem_size], rb->elem_size);
    }
}
void ring_buffer_peek_head(const struct ring_buffer *rb, void *data, uint8_t offset)
{
    ASSERT(offset < ring_buffer_count(rb));
    int16_t offset_idx = ((int16_t)rb->head - 1) - offset;
    if (offset_idx < 0) {
        offset_idx = rb->size + offset_idx;
    }
    memcpy(data, &rb->buffer[(uint8_t)offset_idx * rb->elem_size], rb->elem_size);
}
uint8_t ring_buffer_count(const struct ring_buffer *rb)
{
    if (rb->full) {
        return rb->size;
    } else if (rb->tail <= rb->head) {
        return rb->head - rb->tail;
    } else {
        return (rb->size - rb->tail) + rb->head + 1;
    }
}

bool ring_buffer_full(const struct ring_buffer *rb)
{
    return rb->full;
}
bool ring_buffer_empty(const struct ring_buffer *rb)
{
    return !rb->full && (rb->head == rb->tail);
}