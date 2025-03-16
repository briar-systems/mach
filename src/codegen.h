#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "target.h"

#define CODEGEN_MAX_LABEL_LEN 64
#define CODEGEN_MAX_DIRECTIVE_LEN 32

typedef enum
{
    SECTION_TEXT,   // Code section
    SECTION_DATA,   // Initialized data section
    SECTION_RODATA, // Read-only data section
    SECTION_BSS,    // Uninitialized data section
} CodegenSection;

typedef enum
{
    OPTYPE_NONE,  // No operand
    OPTYPE_REG,   // Register
    OPTYPE_IMM,   // Immediate value
    OPTYPE_MEM,   // Memory reference
    OPTYPE_LABEL, // Label reference
} OperandType;

typedef enum
{
    // x86-64 general purpose registers
    REG_RAX,
    REG_RBX,
    REG_RCX,
    REG_RDX,
    REG_RSI,
    REG_RDI,
    REG_RBP,
    REG_RSP,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,

    // x86 general purpose registers
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_ESI,
    REG_EDI,
    REG_EBP,
    REG_ESP,

    // 16-bit registers
    REG_AX,
    REG_BX,
    REG_CX,
    REG_DX,
    REG_SI,
    REG_DI,
    REG_BP,
    REG_SP,

    // 8-bit registers
    REG_AL,
    REG_BL,
    REG_CL,
    REG_DL,
    REG_AH,
    REG_BH,
    REG_CH,
    REG_DH,

    // x87 floating point registers
    REG_ST0,
    REG_ST1,
    REG_ST2,
    REG_ST3,
    REG_ST4,
    REG_ST5,
    REG_ST6,
    REG_ST7,

    // XMM registers
    REG_XMM0,
    REG_XMM1,
    REG_XMM2,
    REG_XMM3,
    REG_XMM4,
    REG_XMM5,
    REG_XMM6,
    REG_XMM7,
    REG_XMM8,
    REG_XMM9,
    REG_XMM10,
    REG_XMM11,
    REG_XMM12,
    REG_XMM13,
    REG_XMM14,
    REG_XMM15,
} Register;

typedef enum
{
    ADDRMODE_BASE,             // [reg]
    ADDRMODE_DISP,             // [disp]
    ADDRMODE_BASE_DISP,        // [reg+disp]
    ADDRMODE_INDEX_SCALE,      // [reg+reg*scale]
    ADDRMODE_BASE_INDEX_SCALE, // [reg+reg*scale]
    ADDRMODE_FULL,             // [reg+reg*scale+disp]
} AddrMode;

typedef struct
{
    AddrMode mode;
    Register base;
    Register index;
    int scale; // 1, 2, 4, or 8
    int32_t displacement;
} MemOperand;

typedef struct
{
    OperandType type;
    union
    {
        Register reg;
        int64_t imm;
        MemOperand mem;
        char label[CODEGEN_MAX_LABEL_LEN];
    };
    uint8_t size; // Size in bytes
} Operand;

typedef enum
{
    SIZE_BYTE = 1,   // 8-bit
    SIZE_WORD = 2,   // 16-bit
    SIZE_DWORD = 4,  // 32-bit
    SIZE_QWORD = 8,  // 64-bit
    SIZE_OWORD = 16, // 128-bit (xmm)
} OperandSize;

typedef enum
{
    // Data movement
    INS_MOV,
    INS_PUSH,
    INS_POP,
    INS_LEA,
    INS_MOVZX,
    INS_MOVSX,

    // Arithmetic
    INS_ADD,
    INS_SUB,
    INS_MUL,
    INS_DIV,
    INS_INC,
    INS_DEC,
    INS_NEG,
    INS_IMUL,
    INS_IDIV,

    // Logic
    INS_AND,
    INS_OR,
    INS_XOR,
    INS_NOT,
    INS_SHL,
    INS_SHR,
    INS_SAR,
    INS_ROL,
    INS_ROR,

    // Control flow
    INS_JMP,
    INS_JE,
    INS_JNE,
    INS_JZ,
    INS_JNZ,
    INS_JG,
    INS_JGE,
    INS_JL,
    INS_JLE,
    INS_CALL,
    INS_RET,

    // Comparison
    INS_CMP,
    INS_TEST,

    // System
    INS_SYSCALL,
    INS_INT,

    // SSE instructions
    INS_MOVSS,
    INS_ADDSS,
    INS_SUBSS,
    INS_MULSS,
    INS_DIVSS,
    INS_MOVSD,
    INS_ADDSD,
    INS_SUBSD,
    INS_MULSD,
    INS_DIVSD,

    // Special
    INS_NOP,
    INS_HLT,

    // Extended instruction count
    INS_COUNT
} InstructionType;

typedef enum
{
    FIXUP_NONE,
    FIXUP_RELATIVE, // PC-relative address
    FIXUP_ABSOLUTE, // Absolute address
} FixupType;

typedef struct
{
    char label[CODEGEN_MAX_LABEL_LEN];
    size_t offset; // Offset into the code where the fixup is needed
    FixupType type;
    int size; // Size of the fixup (typically 4 or 8 bytes)
} Fixup;

typedef struct
{
    char name[CODEGEN_MAX_LABEL_LEN];
    size_t offset;
    bool is_global;
    bool is_function;
    CodegenSection section;
} ASMSymbol;

typedef struct
{
    // Target architecture and OS
    Target target;

    // Buffer for assembly text output
    char *asm_buffer;
    size_t asm_capacity;
    size_t asm_used;

    // Current section being generated
    CodegenSection current_section;

    // Current function being generated
    char current_function[CODEGEN_MAX_LABEL_LEN];

    // Symbol table for labels and functions
    ASMSymbol *symbols;
    size_t symbol_count;
    size_t symbol_capacity;

    // Fixup table for forward references
    Fixup *fixups;
    size_t fixup_count;
    size_t fixup_capacity;

    // Unique label counter for generating unique labels
    uint32_t label_counter;
} CodeEmitter;

bool codegen_init(CodeEmitter *emitter, Target target);

void codegen_free(CodeEmitter *emitter);
bool codegen_write_asm(CodeEmitter *emitter, const char *filename);
bool codegen_assemble(CodeEmitter *emitter, const char *asm_filename, const char *obj_filename);
bool codegen_link(CodeEmitter *emitter, const char **obj_filenames, int obj_count, const char *exe_filename);
bool codegen_build(CodeEmitter *emitter, const char *asm_filename, const char *exe_filename);

void codegen_switch_section(CodeEmitter *emitter, CodegenSection section);
void codegen_define_global(CodeEmitter *emitter, const char *name);
void codegen_define_function(CodeEmitter *emitter, const char *name, bool is_global);

void codegen_end_function(CodeEmitter *emitter);
void codegen_define_label(CodeEmitter *emitter, const char *name);
void codegen_gen_unique_label(CodeEmitter *emitter, const char *prefix, char *buffer, size_t buffer_size);

void codegen_define_byte(CodeEmitter *emitter, const char *name, uint8_t value);
void codegen_define_word(CodeEmitter *emitter, const char *name, uint16_t value);
void codegen_define_dword(CodeEmitter *emitter, const char *name, uint32_t value);
void codegen_define_qword(CodeEmitter *emitter, const char *name, uint64_t value);
void codegen_define_string(CodeEmitter *emitter, const char *name, const char *str);
void codegen_define_space(CodeEmitter *emitter, const char *name, size_t size);

Operand codegen_reg(Register reg, OperandSize size);
Operand codegen_imm(int64_t value, OperandSize size);
Operand codegen_label(const char *label);
Operand codegen_mem_base(Register base, OperandSize size);
Operand codegen_mem_disp(int32_t disp, OperandSize size);
Operand codegen_mem_base_disp(Register base, int32_t disp, OperandSize size);
Operand codegen_mem_base_index(Register base, Register index, int scale, OperandSize size);
Operand codegen_mem_full(Register base, Register index, int scale, int32_t disp, OperandSize size);

void codegen_emit_directive(CodeEmitter *emitter, const char *format, ...);
void codegen_emit_comment(CodeEmitter *emitter, const char *format, ...);
void codegen_emit_0(CodeEmitter *emitter, InstructionType ins_type);
void codegen_emit_1(CodeEmitter *emitter, InstructionType ins_type, Operand op);
void codegen_emit_2(CodeEmitter *emitter, InstructionType ins_type, Operand op1, Operand op2);
void codegen_emit_3(CodeEmitter *emitter, InstructionType ins_type, Operand op1, Operand op2, Operand op3);
void codegen_emit_raw(CodeEmitter *emitter, const char *format, ...);

void codegen_emit_mov(CodeEmitter *emitter, Operand dest, Operand src);
void codegen_emit_add(CodeEmitter *emitter, Operand dest, Operand src);
void codegen_emit_sub(CodeEmitter *emitter, Operand dest, Operand src);
void codegen_emit_mul(CodeEmitter *emitter, Operand dest, Operand src);
void codegen_emit_cmp(CodeEmitter *emitter, Operand op1, Operand op2);
void codegen_emit_jmp(CodeEmitter *emitter, const char *label);
void codegen_emit_jcc(CodeEmitter *emitter, InstructionType condition, const char *label);
void codegen_emit_call(CodeEmitter *emitter, const char *target);

void codegen_emit_ret(CodeEmitter *emitter);
void codegen_emit_push(CodeEmitter *emitter, Operand op);
void codegen_emit_pop(CodeEmitter *emitter, Operand op);
void codegen_emit_lea(CodeEmitter *emitter, Operand dest, Operand src);

void codegen_emit_prologue(CodeEmitter *emitter, size_t local_bytes);
void codegen_emit_epilogue(CodeEmitter *emitter);

void codegen_emit_int(CodeEmitter *emitter, Operand op);
void codegen_emit_syscall(CodeEmitter *emitter);

void codegen_mangle_name(Target target, const char *name, char *buffer, size_t buffer_size);
void codegen_emit_entry_point(CodeEmitter *emitter, const char *main_func);
bool codegen_resolve_fixups(CodeEmitter *emitter);

bool codegen_generate_test_program(CodeEmitter *emitter, int return_value);
bool codegen_generate_add_test(CodeEmitter *emitter);

#endif // CODEGEN_H
