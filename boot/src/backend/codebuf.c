#include "backend/codebuf.h"
#include <stdlib.h>
#include <string.h>

void codebuf_init(CodeBuffer *buf)
{
    buf->data     = NULL;
    buf->size     = 0;
    buf->capacity = 0;
}

void codebuf_free(CodeBuffer *buf)
{
    if (buf->data)
    {
        free(buf->data);
    }
    buf->data     = NULL;
    buf->size     = 0;
    buf->capacity = 0;
}

bool codebuf_reserve(CodeBuffer *buf, size_t additional)
{
    if (buf->size + additional <= buf->capacity)
    {
        return true;
    }

    size_t new_capacity = buf->capacity == 0 ? 1024 : buf->capacity;
    while (new_capacity < buf->size + additional)
    {
        new_capacity *= 2;
    }

    uint8_t *new_data = realloc(buf->data, new_capacity);
    if (!new_data)
    {
        return false;
    }

    buf->data     = new_data;
    buf->capacity = new_capacity;
    return true;
}

void codebuf_emit_byte(CodeBuffer *buf, uint8_t byte)
{
    if (!codebuf_reserve(buf, 1))
    {
        return;
    }
    buf->data[buf->size++] = byte;
}

void codebuf_emit_bytes(CodeBuffer *buf, const uint8_t *bytes, size_t count)
{
    if (!codebuf_reserve(buf, count))
    {
        return;
    }
    memcpy(buf->data + buf->size, bytes, count);
    buf->size += count;
}

void codebuf_emit_u32(CodeBuffer *buf, uint32_t value)
{
    if (!codebuf_reserve(buf, 4))
    {
        return;
    }
    buf->data[buf->size++] = (uint8_t)(value & 0xFF);
    buf->data[buf->size++] = (uint8_t)((value >> 8) & 0xFF);
    buf->data[buf->size++] = (uint8_t)((value >> 16) & 0xFF);
    buf->data[buf->size++] = (uint8_t)((value >> 24) & 0xFF);
}

void codebuf_emit_u64(CodeBuffer *buf, uint64_t value)
{
    if (!codebuf_reserve(buf, 8))
    {
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        buf->data[buf->size++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

size_t codebuf_offset(const CodeBuffer *buf)
{
    return buf ? buf->size : 0;
}
