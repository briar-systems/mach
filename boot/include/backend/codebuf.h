#ifndef BACKEND_CODEBUF_H
#define BACKEND_CODEBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CodeBuffer
{
    uint8_t *data;
    size_t   size;
    size_t   capacity;
} CodeBuffer;

void   codebuf_init(CodeBuffer *buf);
void   codebuf_free(CodeBuffer *buf);
bool   codebuf_reserve(CodeBuffer *buf, size_t additional);
void   codebuf_emit_byte(CodeBuffer *buf, uint8_t byte);
void   codebuf_emit_bytes(CodeBuffer *buf, const uint8_t *bytes, size_t count);
void   codebuf_emit_u32(CodeBuffer *buf, uint32_t value);
void   codebuf_emit_u64(CodeBuffer *buf, uint64_t value);
size_t codebuf_offset(const CodeBuffer *buf);

#endif // BACKEND_CODEBUF_H
