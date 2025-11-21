#ifndef X86_64_H
#define X86_64_H

#include "mir.h"
#include <stdint.h>
#include <stdio.h>

// code buffer for emitting instructions
typedef struct CodeBuffer
{
    uint8_t *data;
    size_t   size;
    size_t   capacity;
} CodeBuffer;

// label resolution entry
typedef struct LabelRef
{
    const char *label;       // label name
    size_t      offset;      // offset in code buffer to patch
    bool        is_rel32;    // true = relative offset, false = absolute
    struct LabelRef *next;
} LabelRef;

// label definition
typedef struct LabelDef
{
    const char *label;       // label name
    size_t      offset;      // offset in code buffer
    struct LabelDef *next;
} LabelDef;

// x86_64 code generator context
typedef struct X86_64Context
{
    CodeBuffer  code;           // emitted code
    LabelRef   *label_refs;     // unresolved label references
    LabelDef   *label_defs;     // defined labels
    uint32_t   *vreg_to_preg;   // virtual register allocation map
    uint32_t    vreg_count;     // number of virtual registers
} X86_64Context;

// context management
X86_64Context *x86_64_context_create(void);
void           x86_64_context_destroy(X86_64Context *ctx);

// code buffer operations
void     codebuf_init(CodeBuffer *buf);
void     codebuf_free(CodeBuffer *buf);
void     codebuf_emit_byte(CodeBuffer *buf, uint8_t byte);
void     codebuf_emit_bytes(CodeBuffer *buf, const uint8_t *bytes, size_t count);
void     codebuf_emit_u32(CodeBuffer *buf, uint32_t value);
void     codebuf_patch_u32(CodeBuffer *buf, size_t offset, uint32_t value);
size_t   codebuf_offset(CodeBuffer *buf);

// label management
void     x86_64_define_label(X86_64Context *ctx, const char *label, size_t offset);
void     x86_64_add_label_ref(X86_64Context *ctx, const char *label, size_t offset, bool is_rel32);
bool     x86_64_resolve_labels(X86_64Context *ctx);
LabelDef *x86_64_find_label(X86_64Context *ctx, const char *label);

// register allocation (simple for now)
void     x86_64_allocate_registers(X86_64Context *ctx, MirModule *module);
uint8_t  x86_64_get_preg(X86_64Context *ctx, uint32_t vreg_id);

// instruction encoding
void x86_64_encode_instruction(X86_64Context *ctx, MirInstruction *inst);

// high-level code generation
bool x86_64_generate_code(MirModule *module, const char *output_path);

// ELF generation
bool x86_64_generate_elf(X86_64Context *ctx, MirModule *module, const char *output_path);

#endif // X86_64_H
