#include "codegen.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define CODEGEN_INITIAL_BUFFER_SIZE 8192
#define CODEGEN_SYMBOL_INITIAL_CAPACITY 64
#define CODEGEN_FIXUP_INITIAL_CAPACITY 64

// instruction names mapping
static const char *instruction_names[] = {
    [INS_MOV] = "mov",
    [INS_PUSH] = "push",
    [INS_POP] = "pop",
    [INS_LEA] = "lea",
    [INS_MOVZX] = "movzx",
    [INS_MOVSX] = "movsx",
    [INS_ADD] = "add",
    [INS_SUB] = "sub",
    [INS_MUL] = "mul",
    [INS_DIV] = "div",
    [INS_INC] = "inc",
    [INS_DEC] = "dec",
    [INS_NEG] = "neg",
    [INS_IMUL] = "imul",
    [INS_IDIV] = "idiv",
    [INS_AND] = "and",
    [INS_OR] = "or",
    [INS_XOR] = "xor",
    [INS_NOT] = "not",
    [INS_SHL] = "shl",
    [INS_SHR] = "shr",
    [INS_SAR] = "sar",
    [INS_ROL] = "rol",
    [INS_ROR] = "ror",
    [INS_JMP] = "jmp",
    [INS_JE] = "je",
    [INS_JNE] = "jne",
    [INS_JZ] = "jz",
    [INS_JNZ] = "jnz",
    [INS_JG] = "jg",
    [INS_JGE] = "jge",
    [INS_JL] = "jl",
    [INS_JLE] = "jle",
    [INS_CALL] = "call",
    [INS_RET] = "ret",
    [INS_CMP] = "cmp",
    [INS_TEST] = "test",
    [INS_SYSCALL] = "syscall",
    [INS_INT] = "int",
    [INS_MOVSS] = "movss",
    [INS_ADDSS] = "addss",
    [INS_SUBSS] = "subss",
    [INS_MULSS] = "mulss",
    [INS_DIVSS] = "divss",
    [INS_MOVSD] = "movsd",
    [INS_ADDSD] = "addsd",
    [INS_SUBSD] = "subsd",
    [INS_MULSD] = "mulsd",
    [INS_DIVSD] = "divsd",
    [INS_NOP] = "nop",
    [INS_HLT] = "hlt",
};

static const char *register_names[] = {
    [REG_RAX] = "rax",
    [REG_RBX] = "rbx",
    [REG_RCX] = "rcx",
    [REG_RDX] = "rdx",
    [REG_RSI] = "rsi",
    [REG_RDI] = "rdi",
    [REG_RBP] = "rbp",
    [REG_RSP] = "rsp",
    [REG_R8] = "r8",
    [REG_R9] = "r9",
    [REG_R10] = "r10",
    [REG_R11] = "r11",
    [REG_R12] = "r12",
    [REG_R13] = "r13",
    [REG_R14] = "r14",
    [REG_R15] = "r15",
    [REG_EAX] = "eax",
    [REG_EBX] = "ebx",
    [REG_ECX] = "ecx",
    [REG_EDX] = "edx",
    [REG_ESI] = "esi",
    [REG_EDI] = "edi",
    [REG_EBP] = "ebp",
    [REG_ESP] = "esp",
    [REG_AX] = "ax",
    [REG_BX] = "bx",
    [REG_CX] = "cx",
    [REG_DX] = "dx",
    [REG_SI] = "si",
    [REG_DI] = "di",
    [REG_BP] = "bp",
    [REG_SP] = "sp",
    [REG_AL] = "al",
    [REG_BL] = "bl",
    [REG_CL] = "cl",
    [REG_DL] = "dl",
    [REG_AH] = "ah",
    [REG_BH] = "bh",
    [REG_CH] = "ch",
    [REG_DH] = "dh",
    [REG_ST0] = "st0",
    [REG_ST1] = "st1",
    [REG_ST2] = "st2",
    [REG_ST3] = "st3",
    [REG_ST4] = "st4",
    [REG_ST5] = "st5",
    [REG_ST6] = "st6",
    [REG_ST7] = "st7",
    [REG_XMM0] = "xmm0",
    [REG_XMM1] = "xmm1",
    [REG_XMM2] = "xmm2",
    [REG_XMM3] = "xmm3",
    [REG_XMM4] = "xmm4",
    [REG_XMM5] = "xmm5",
    [REG_XMM6] = "xmm6",
    [REG_XMM7] = "xmm7",
    [REG_XMM8] = "xmm8",
    [REG_XMM9] = "xmm9",
    [REG_XMM10] = "xmm10",
    [REG_XMM11] = "xmm11",
    [REG_XMM12] = "xmm12",
    [REG_XMM13] = "xmm13",
    [REG_XMM14] = "xmm14",
    [REG_XMM15] = "xmm15",
};

static const char *section_names[] = {
    [SECTION_TEXT] = ".text",
    [SECTION_DATA] = ".data",
    [SECTION_RODATA] = ".rodata",
    [SECTION_BSS] = ".bss",
};

// helpers
static void ensure_buffer_capacity(CodeEmitter *emitter, size_t additional_size) {
    if (emitter->asm_used + additional_size > emitter->asm_capacity) {
        size_t new_capacity = emitter->asm_capacity * 2;
        if (new_capacity < emitter->asm_used + additional_size) {
            new_capacity = emitter->asm_used + additional_size + CODEGEN_INITIAL_BUFFER_SIZE;
        }
        
        char *new_buffer = realloc(emitter->asm_buffer, new_capacity);
        if (!new_buffer) {
            fprintf(stderr, "error: failed to allocate memory for code buffer\n");
            exit(1);
        }
        
        emitter->asm_buffer = new_buffer;
        emitter->asm_capacity = new_capacity;
    }
}

static void ensure_symbol_capacity(CodeEmitter *emitter) {
    if (emitter->symbol_count >= emitter->symbol_capacity) {
        size_t new_capacity = emitter->symbol_capacity * 2;
        if (new_capacity == 0) {
            new_capacity = CODEGEN_SYMBOL_INITIAL_CAPACITY;
        }
        
        ASMSymbol *new_symbols = realloc(emitter->symbols, new_capacity * sizeof(ASMSymbol));
        if (!new_symbols) {
            fprintf(stderr, "error: failed to allocate memory for symbols\n");
            exit(1);
        }
        
        emitter->symbols = new_symbols;
        emitter->symbol_capacity = new_capacity;
    }
}

static void ensure_fixup_capacity(CodeEmitter *emitter) {
    if (emitter->fixup_count >= emitter->fixup_capacity) {
        size_t new_capacity = emitter->fixup_capacity * 2;
        if (new_capacity == 0) {
            new_capacity = CODEGEN_FIXUP_INITIAL_CAPACITY;
        }
        
        Fixup *new_fixups = realloc(emitter->fixups, new_capacity * sizeof(Fixup));
        if (!new_fixups) {
            fprintf(stderr, "error: failed to allocate memory for fixups\n");
            exit(1);
        }
        
        emitter->fixups = new_fixups;
        emitter->fixup_capacity = new_capacity;
    }
}

static void append_string(CodeEmitter *emitter, const char *str) {
    size_t len = strlen(str);
    ensure_buffer_capacity(emitter, len + 1);
    memcpy(emitter->asm_buffer + emitter->asm_used, str, len + 1);
    emitter->asm_used += len;
}

static void append_formatted(CodeEmitter *emitter, const char *format, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    
    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    ensure_buffer_capacity(emitter, required_size);
    
    int written = vsnprintf(emitter->asm_buffer + emitter->asm_used, required_size, format, args_copy);
    emitter->asm_used += written;
    
    va_end(args_copy);
    va_end(args);
}

static const char *get_register_name(CodeEmitter *emitter, Register reg) {
    return register_names[reg];
}

static const char *get_section_name(CodeEmitter *emitter, CodegenSection section) {
    return section_names[section];
}

static ASMSymbol *find_symbol(CodeEmitter *emitter, const char *name) {
    for (size_t i = 0; i < emitter->symbol_count; i++) {
        if (strcmp(emitter->symbols[i].name, name) == 0) {
            return &emitter->symbols[i];
        }
    }
    return NULL;
}

static void add_symbol(CodeEmitter *emitter, const char *name, size_t offset, bool is_global, bool is_function, CodegenSection section) {
    ensure_symbol_capacity(emitter);
    ASMSymbol *sym = &emitter->symbols[emitter->symbol_count++];
    strncpy(sym->name, name, CODEGEN_MAX_LABEL_LEN - 1);
    sym->name[CODEGEN_MAX_LABEL_LEN - 1] = '\0';
    sym->offset = offset;
    sym->is_global = is_global;
    sym->is_function = is_function;
    sym->section = section;
}

static void add_fixup(CodeEmitter *emitter, const char *label, size_t offset, FixupType type, int size) {
    ensure_fixup_capacity(emitter);
    Fixup *fixup = &emitter->fixups[emitter->fixup_count++];
    strncpy(fixup->label, label, CODEGEN_MAX_LABEL_LEN - 1);
    fixup->label[CODEGEN_MAX_LABEL_LEN - 1] = '\0';
    fixup->offset = offset;
    fixup->type = type;
    fixup->size = size;
}

static void append_operand(CodeEmitter *emitter, Operand op) {
    switch (op.type) {
        case OPTYPE_REG:
            append_string(emitter, get_register_name(emitter, op.reg));
            break;
            
        case OPTYPE_IMM:
            append_formatted(emitter, "%lld", op.imm);
            break;
            
        case OPTYPE_LABEL:
            append_string(emitter, op.label);
            break;
            
        case OPTYPE_MEM: {
            // Intel syntax: [base + index*scale + disp]
            append_string(emitter, "[");
            bool first = true;
            
            if (op.mem.base != 0) {
                append_string(emitter, get_register_name(emitter, op.mem.base));
                first = false;
            }
            
            if (op.mem.index != 0) {
                if (!first) append_string(emitter, " + ");
                append_string(emitter, get_register_name(emitter, op.mem.index));
                if (op.mem.scale > 1) {
                    append_formatted(emitter, "*%d", op.mem.scale);
                }
                first = false;
            }
            
            if (op.mem.displacement != 0 || (op.mem.base == 0 && op.mem.index == 0)) {
                if (!first) {
                    if (op.mem.displacement > 0)
                        append_string(emitter, " + ");
                    else {
                        append_string(emitter, " - ");
                        append_formatted(emitter, "%d", -op.mem.displacement);
                        break;
                    }
                }
                append_formatted(emitter, "%d", op.mem.displacement);
            }
            
            append_string(emitter, "]");
            break;
        }
        
        default:
            append_string(emitter, "???");
            break;
    }
}

// public api implementation
bool codegen_init(CodeEmitter *emitter, Target target) {
    memset(emitter, 0, sizeof(CodeEmitter));
    
    emitter->target = target;
    
    emitter->asm_capacity = CODEGEN_INITIAL_BUFFER_SIZE;
    emitter->asm_buffer = malloc(emitter->asm_capacity);
    if (!emitter->asm_buffer) {
        fprintf(stderr, "error: failed to allocate initial code buffer\n");
        return false;
    }
    
    emitter->asm_used = 0;
    emitter->current_section = SECTION_TEXT;
    emitter->symbol_count = 0;
    emitter->symbol_capacity = 0;
    emitter->fixup_count = 0;
    emitter->fixup_capacity = 0;
    emitter->label_counter = 0;
    
    codegen_emit_directive(emitter, ".file \"generated.s\"");
    codegen_emit_directive(emitter, ".intel_syntax noprefix");
    
    // switch to text section by default
    codegen_switch_section(emitter, SECTION_TEXT);
    
    return true;
}

void codegen_free(CodeEmitter *emitter) {
    if (emitter->asm_buffer) {
        free(emitter->asm_buffer);
        emitter->asm_buffer = NULL;
    }
    
    if (emitter->symbols) {
        free(emitter->symbols);
        emitter->symbols = NULL;
    }
    
    if (emitter->fixups) {
        free(emitter->fixups);
        emitter->fixups = NULL;
    }
}

bool codegen_write_asm(CodeEmitter *emitter, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "error: failed to open '%s' for writing: %s\n", filename, strerror(errno));
        return false;
    }
    
    size_t written = fwrite(emitter->asm_buffer, 1, emitter->asm_used, file);
    fclose(file);
    
    return written == emitter->asm_used;
}

bool codegen_assemble(CodeEmitter *emitter, const char *asm_filename, const char *obj_filename) {
    char cmd[512];
    int status;
    
    snprintf(cmd, sizeof(cmd), "as -o %s %s --gstabs", obj_filename, asm_filename);
    
    status = system(cmd);
    
    return status == 0;
}

bool codegen_link(CodeEmitter *emitter, const char **obj_filenames, int obj_count, const char *exe_filename) {
    char cmd[4096] = {0};
    int cmd_pos = 0;
    int status;
    
    // choose appropriate linker based on target
    const char *ld_cmd;
    if (emitter->target.os == OS_WINDOWS) {
        ld_cmd = "link";  // Windows linker
    } else if (emitter->target.os == OS_MACOS) {
        ld_cmd = "ld";  // macOS linker
    } else {
        ld_cmd = "ld";  // default to GNU ld
    }
    
    // start building command
    cmd_pos += snprintf(cmd + cmd_pos, sizeof(cmd) - cmd_pos, "%s -o %s", ld_cmd, exe_filename);
    
    // add object files
    for (int i = 0; i < obj_count; i++) {
        cmd_pos += snprintf(cmd + cmd_pos, sizeof(cmd) - cmd_pos, " %s", obj_filenames[i]);
    }
    
    status = system(cmd);
    
    return status == 0;
}

bool codegen_build(CodeEmitter *emitter, const char *asm_filename, const char *exe_filename) {
    // create temporary object file name
    char obj_filename[256];
    snprintf(obj_filename, sizeof(obj_filename), "%s.o", exe_filename);
    
    // assemble
    if (!codegen_assemble(emitter, asm_filename, obj_filename)) {
        fprintf(stderr, "error: assembly failed\n");
        return false;
    }
    
    // link
    const char *objs[] = {obj_filename};
    if (!codegen_link(emitter, objs, 1, exe_filename)) {
        fprintf(stderr, "error: linking failed\n");
        unlink(obj_filename);
        return false;
    }
    
    // cleanup object file
    unlink(obj_filename);
    
    // make executable
    chmod(exe_filename, 0755);
    
    return true;
}

void codegen_switch_section(CodeEmitter *emitter, CodegenSection section) {
    if (emitter->current_section == section) {
        return;
    }
    
    emitter->current_section = section;
    codegen_emit_directive(emitter, "%s", get_section_name(emitter, section));
}

void codegen_define_global(CodeEmitter *emitter, const char *name) {
    codegen_emit_directive(emitter, ".global %s", name);
}

void codegen_define_function(CodeEmitter *emitter, const char *name, bool is_global) {
    codegen_switch_section(emitter, SECTION_TEXT);
    
    if (is_global) {
        codegen_define_global(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".type %s, @function", name);
    strncpy(emitter->current_function, name, CODEGEN_MAX_LABEL_LEN - 1);
    emitter->current_function[CODEGEN_MAX_LABEL_LEN - 1] = '\0';
    codegen_define_label(emitter, name);
}

void codegen_end_function(CodeEmitter *emitter) {
    if (emitter->current_function[0] != '\0') {
        codegen_emit_directive(emitter, ".size %s, .-%s", emitter->current_function, emitter->current_function);
        emitter->current_function[0] = '\0';
    }
}

void codegen_define_label(CodeEmitter *emitter, const char *name) {
    append_formatted(emitter, "%s:\n", name);
    add_symbol(emitter, name, emitter->asm_used - strlen(name) - 2, false, false, emitter->current_section);
}

void codegen_gen_unique_label(CodeEmitter *emitter, const char *prefix, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, ".L%s_%u", prefix, emitter->label_counter++);
}

void codegen_define_byte(CodeEmitter *emitter, const char *name, uint8_t value) {
    codegen_switch_section(emitter, SECTION_DATA);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".byte %u", value);
}

void codegen_define_word(CodeEmitter *emitter, const char *name, uint16_t value) {
    codegen_switch_section(emitter, SECTION_DATA);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".word %u", value);
}

void codegen_define_dword(CodeEmitter *emitter, const char *name, uint32_t value) {
    codegen_switch_section(emitter, SECTION_DATA);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".long %u", value);
}

void codegen_define_qword(CodeEmitter *emitter, const char *name, uint64_t value) {
    codegen_switch_section(emitter, SECTION_DATA);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".quad %llu", value);
}

void codegen_define_string(CodeEmitter *emitter, const char *name, const char *str) {
    codegen_switch_section(emitter, SECTION_DATA);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".string \"%s\"", str);
}

void codegen_define_space(CodeEmitter *emitter, const char *name, size_t size) {
    codegen_switch_section(emitter, SECTION_BSS);
    
    if (name) {
        codegen_define_label(emitter, name);
    }
    
    codegen_emit_directive(emitter, ".zero %zu", size);
}

Operand codegen_reg(Register reg, OperandSize size) {
    Operand op;
    op.type = OPTYPE_REG;
    op.reg = reg;
    op.size = size;
    return op;
}

Operand codegen_imm(int64_t value, OperandSize size) {
    Operand op;
    op.type = OPTYPE_IMM;
    op.imm = value;
    op.size = size;
    return op;
}

Operand codegen_label(const char *label) {
    Operand op;
    op.type = OPTYPE_LABEL;
    strncpy(op.label, label, CODEGEN_MAX_LABEL_LEN - 1);
    op.label[CODEGEN_MAX_LABEL_LEN - 1] = '\0';
    op.size = 0;  // size doesn't apply to labels
    return op;
}

Operand codegen_label_addr(const char *label, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_DISP;  // special case for label references
    op.mem.base = 0;
    op.mem.index = 0;
    op.mem.scale = 0;
    op.mem.displacement = 0;
    strncpy(op.label, label, CODEGEN_MAX_LABEL_LEN - 1);
    op.label[CODEGEN_MAX_LABEL_LEN - 1] = '\0';
    op.size = size;
    return op;
}

Operand codegen_mem_base(Register base, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_BASE;
    op.mem.base = base;
    op.mem.index = 0;
    op.mem.scale = 0;
    op.mem.displacement = 0;
    op.size = size;
    return op;
}

Operand codegen_mem_disp(int32_t disp, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_DISP;
    op.mem.base = 0;
    op.mem.index = 0;
    op.mem.scale = 0;
    op.mem.displacement = disp;
    op.size = size;
    return op;
}

Operand codegen_mem_base_disp(Register base, int32_t disp, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_BASE_DISP;
    op.mem.base = base;
    op.mem.index = 0;
    op.mem.scale = 0;
    op.mem.displacement = disp;
    op.size = size;
    return op;
}

Operand codegen_mem_base_index(Register base, Register index, int scale, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_BASE_INDEX_SCALE;
    op.mem.base = base;
    op.mem.index = index;
    op.mem.scale = scale;
    op.mem.displacement = 0;
    op.size = size;
    return op;
}

Operand codegen_mem_full(Register base, Register index, int scale, int32_t disp, OperandSize size) {
    Operand op;
    op.type = OPTYPE_MEM;
    op.mem.mode = ADDRMODE_FULL;
    op.mem.base = base;
    op.mem.index = index;
    op.mem.scale = scale;
    op.mem.displacement = disp;
    op.size = size;
    return op;
}

void codegen_emit_directive(CodeEmitter *emitter, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    ensure_buffer_capacity(emitter, strlen(format) + 100);  // rough estimate
    append_string(emitter, "\t");
    
    va_list args_copy;
    va_copy(args_copy, args);
    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    ensure_buffer_capacity(emitter, required_size);
    
    int written = vsnprintf(emitter->asm_buffer + emitter->asm_used, required_size, format, args_copy);
    emitter->asm_used += written;
    
    va_end(args_copy);
    va_end(args);
    
    append_string(emitter, "\n");
}

void codegen_emit_comment(CodeEmitter *emitter, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    ensure_buffer_capacity(emitter, strlen(format) + 100);  // rough estimate
    append_string(emitter, "\t# ");
    
    va_list args_copy;
    va_copy(args_copy, args);
    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    ensure_buffer_capacity(emitter, required_size);
    
    int written = vsnprintf(emitter->asm_buffer + emitter->asm_used, required_size, format, args_copy);
    emitter->asm_used += written;
    
    va_end(args_copy);
    va_end(args);
    
    append_string(emitter, "\n");
}

void codegen_emit_0(CodeEmitter *emitter, InstructionType ins_type) {
    const char *ins_name = instruction_names[ins_type];
    append_formatted(emitter, "\t%s\n", ins_name);
}

void codegen_emit_1(CodeEmitter *emitter, InstructionType ins_type, Operand op) {
    const char *ins_name = instruction_names[ins_type];
    append_string(emitter, "\t");
    append_string(emitter, ins_name);
    append_string(emitter, " ");
    append_operand(emitter, op);
    append_string(emitter, "\n");
}

void codegen_emit_2(CodeEmitter *emitter, InstructionType ins_type, Operand op1, Operand op2) {
    const char *ins_name = instruction_names[ins_type];
    
    append_string(emitter, "\t");
    append_string(emitter, ins_name);
    append_string(emitter, " ");
    
    append_operand(emitter, op1);
    append_string(emitter, ", ");
    append_operand(emitter, op2);
    
    append_string(emitter, "\n");
}

void codegen_emit_3(CodeEmitter *emitter, InstructionType ins_type, Operand op1, Operand op2, Operand op3) {
    const char *ins_name = instruction_names[ins_type];
    
    append_string(emitter, "\t");
    append_string(emitter, ins_name);
    append_string(emitter, " ");
    
    append_operand(emitter, op1);
    append_string(emitter, ", ");
    append_operand(emitter, op2);
    append_string(emitter, ", ");
    append_operand(emitter, op3);
    
    append_string(emitter, "\n");
}

void codegen_emit_raw(CodeEmitter *emitter, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    va_list args_copy;
    va_copy(args_copy, args);
    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    ensure_buffer_capacity(emitter, required_size);
    
    int written = vsnprintf(emitter->asm_buffer + emitter->asm_used, required_size, format, args_copy);
    emitter->asm_used += written;
    
    va_end(args_copy);
    va_end(args);
    
    append_string(emitter, "\n");
}

// convenience methods
void codegen_emit_mov(CodeEmitter *emitter, Operand dest, Operand src) {
    codegen_emit_2(emitter, INS_MOV, dest, src);
}

void codegen_emit_add(CodeEmitter *emitter, Operand dest, Operand src) {
    codegen_emit_2(emitter, INS_ADD, dest, src);
}

void codegen_emit_sub(CodeEmitter *emitter, Operand dest, Operand src) {
    codegen_emit_2(emitter, INS_SUB, dest, src);
}

void codegen_emit_mul(CodeEmitter *emitter, Operand dest, Operand src) {
    codegen_emit_2(emitter, INS_MUL, dest, src);
}

void codegen_emit_cmp(CodeEmitter *emitter, Operand op1, Operand op2) {
    codegen_emit_2(emitter, INS_CMP, op1, op2);
}

void codegen_emit_jmp(CodeEmitter *emitter, const char *label) {
    Operand op = codegen_label(label);
    codegen_emit_1(emitter, INS_JMP, op);
}

void codegen_emit_jcc(CodeEmitter *emitter, InstructionType condition, const char *label) {
    Operand op = codegen_label(label);
    codegen_emit_1(emitter, condition, op);
}

void codegen_emit_call(CodeEmitter *emitter, const char *target) {
    Operand op = codegen_label(target);
    codegen_emit_1(emitter, INS_CALL, op);
}

void codegen_emit_ret(CodeEmitter *emitter) {
    codegen_emit_0(emitter, INS_RET);
}

void codegen_emit_push(CodeEmitter *emitter, Operand op) {
    codegen_emit_1(emitter, INS_PUSH, op);
}

void codegen_emit_pop(CodeEmitter *emitter, Operand op) {
    codegen_emit_1(emitter, INS_POP, op);
}

void codegen_emit_lea(CodeEmitter *emitter, Operand dest, Operand src) {
    if (src.type == OPTYPE_LABEL && dest.type == OPTYPE_REG) {
        // for string references, use offset form
        codegen_emit_raw(emitter, "\tlea %s, %s", 
                      get_register_name(emitter, dest.reg),
                      src.label);
        return;
    }
    
    // regular LEA
    codegen_emit_2(emitter, INS_LEA, dest, src);
}

void codegen_emit_prologue(CodeEmitter *emitter, size_t local_bytes) {
    // align local_bytes to the stack alignment
    if (local_bytes % emitter->target.stack_alignment != 0) {
        local_bytes = (local_bytes + emitter->target.stack_alignment - 1) & 
                      ~(emitter->target.stack_alignment - 1);
    }
    
    // x86-64 prologue
    if (emitter->target.arch == ARCH_X86_64) {
        codegen_emit_push(emitter, codegen_reg(REG_RBP, SIZE_QWORD));
        codegen_emit_mov(emitter, codegen_reg(REG_RBP, SIZE_QWORD), codegen_reg(REG_RSP, SIZE_QWORD));
        
        if (local_bytes > 0) {
            codegen_emit_sub(emitter, codegen_reg(REG_RSP, SIZE_QWORD), codegen_imm(local_bytes, SIZE_QWORD));
        }
    }
    // x86 prologue
    else if (emitter->target.arch == ARCH_X86) {
        codegen_emit_push(emitter, codegen_reg(REG_EBP, SIZE_DWORD));
        codegen_emit_mov(emitter, codegen_reg(REG_EBP, SIZE_DWORD), codegen_reg(REG_ESP, SIZE_DWORD));
        
        if (local_bytes > 0) {
            codegen_emit_sub(emitter, codegen_reg(REG_ESP, SIZE_DWORD), codegen_imm(local_bytes, SIZE_DWORD));
        }
    }
}

void codegen_emit_epilogue(CodeEmitter *emitter) {
    // x86-64 epilogue
    if (emitter->target.arch == ARCH_X86_64) {
        codegen_emit_mov(emitter, codegen_reg(REG_RSP, SIZE_QWORD), codegen_reg(REG_RBP, SIZE_QWORD));
        codegen_emit_pop(emitter, codegen_reg(REG_RBP, SIZE_QWORD));
        codegen_emit_ret(emitter);
    }
    // x86 epilogue
    else if (emitter->target.arch == ARCH_X86) {
        codegen_emit_mov(emitter, codegen_reg(REG_ESP, SIZE_DWORD), codegen_reg(REG_EBP, SIZE_DWORD));
        codegen_emit_pop(emitter, codegen_reg(REG_EBP, SIZE_DWORD));
        codegen_emit_ret(emitter);
    }
}

void codegen_emit_int(CodeEmitter *emitter, Operand op) {
    codegen_emit_1(emitter, INS_INT, op);
}

void codegen_emit_syscall(CodeEmitter *emitter) {
    codegen_emit_0(emitter, INS_SYSCALL);
}

void codegen_mangle_name(Target target, const char *name, char *buffer, size_t buffer_size) {
    if (target.os == OS_WINDOWS) {
        // on windows, C functions are often prefixed with underscore
        if (target.arch == ARCH_X86) {
            snprintf(buffer, buffer_size, "_%s", name);
        } else {
            // x64 doesn't use underscore prefix
            snprintf(buffer, buffer_size, "%s", name);
        }
    } else {
        // unix systems generally don't mangle C names
        snprintf(buffer, buffer_size, "%s", name);
    }
}

void codegen_emit_entry_point(CodeEmitter *emitter, const char *main_func) {
    // generate standard entry point (_start) that calls main
    codegen_switch_section(emitter, SECTION_TEXT);
    
    // define _start as global
    codegen_define_global(emitter, "_start");
    
    // define the entry point
    codegen_define_label(emitter, "_start");
    
    if (emitter->target.arch == ARCH_X86_64) {
        if (emitter->target.os == OS_LINUX) {
            // linux x86-64 entry sequence
            // get args from stack and call main
            codegen_emit_mov(emitter, codegen_reg(REG_RDI, SIZE_QWORD), codegen_mem_base(REG_RSP, SIZE_QWORD));
            codegen_emit_lea(emitter, codegen_reg(REG_RSI, SIZE_QWORD), codegen_mem_base_disp(REG_RSP, 8, SIZE_QWORD));
            codegen_emit_call(emitter, main_func);
            
            // exit with main's return value
            codegen_emit_mov(emitter, codegen_reg(REG_RDI, SIZE_QWORD), codegen_reg(REG_RAX, SIZE_QWORD));
            codegen_emit_mov(emitter, codegen_reg(REG_RAX, SIZE_QWORD), codegen_imm(60, SIZE_QWORD)); // sys_exit
            codegen_emit_syscall(emitter);
        }
        else if (emitter->target.os == OS_WINDOWS) {
            // windows x64 doesn't really use _start, but we'll define it
            codegen_emit_call(emitter, main_func);
            codegen_emit_mov(emitter, codegen_reg(REG_RCX, SIZE_QWORD), codegen_reg(REG_RAX, SIZE_QWORD));
            codegen_emit_call(emitter, "ExitProcess");
        }
    }
    else if (emitter->target.arch == ARCH_X86) {
        if (emitter->target.os == OS_LINUX) {
            // linux x86 entry sequence
            codegen_emit_mov(emitter, codegen_reg(REG_EAX, SIZE_DWORD), codegen_mem_base_disp(REG_ESP, 0, SIZE_DWORD));
            codegen_emit_lea(emitter, codegen_reg(REG_EBX, SIZE_DWORD), codegen_mem_base_disp(REG_ESP, 4, SIZE_DWORD));
            codegen_emit_push(emitter, codegen_reg(REG_EBX, SIZE_DWORD));
            codegen_emit_push(emitter, codegen_reg(REG_EAX, SIZE_DWORD));
            codegen_emit_call(emitter, main_func);
            
            // exit with main's return value
            codegen_emit_mov(emitter, codegen_reg(REG_EBX, SIZE_DWORD), codegen_reg(REG_EAX, SIZE_DWORD));
            codegen_emit_mov(emitter, codegen_reg(REG_EAX, SIZE_DWORD), codegen_imm(1, SIZE_DWORD)); // sys_exit
            codegen_emit_int(emitter, codegen_imm(0x80, SIZE_BYTE));
        }
    }
}

bool codegen_resolve_fixups(CodeEmitter *emitter) {
    bool all_resolved = true;
    
    for (size_t i = 0; i < emitter->fixup_count; i++) {
        Fixup *fixup = &emitter->fixups[i];
        ASMSymbol *sym = find_symbol(emitter, fixup->label);
        
        if (sym) {
            // placeholder - in a real system, we'd patch the assembly or
            // leave it for the assembler to handle the references
            codegen_emit_comment(emitter, "fixup for %s at offset %zu", fixup->label, fixup->offset);
        } else {
            fprintf(stderr, "error: unresolved reference to '%s'\n", fixup->label);
            all_resolved = false;
        }
    }
    
    return all_resolved;
}

bool codegen_generate_test_program(CodeEmitter *emitter, int return_value) {
    // define a minimal program that returns a constant
    codegen_emit_entry_point(emitter, "main");
    
    // define the main function
    codegen_define_function(emitter, "main", true);
    codegen_emit_prologue(emitter, 0);
    
    // return the specified value
    if (emitter->target.arch == ARCH_X86_64) {
        codegen_emit_mov(emitter, codegen_reg(REG_RAX, SIZE_QWORD), codegen_imm(return_value, SIZE_QWORD));
    } else {
        codegen_emit_mov(emitter, codegen_reg(REG_EAX, SIZE_DWORD), codegen_imm(return_value, SIZE_DWORD));
    }
    
    codegen_emit_epilogue(emitter);
    codegen_end_function(emitter);
    
    return true;
}

bool codegen_generate_add_test(CodeEmitter *emitter) {
    // define a function that adds two numbers
    codegen_define_function(emitter, "add_numbers", true);
    if (emitter->target.arch == ARCH_X86_64) {
        // function uses x86-64 calling convention
        // parameters in rdi, rsi; return value in rax
        codegen_emit_prologue(emitter, 0);
        codegen_emit_mov(emitter, codegen_reg(REG_RAX, SIZE_QWORD), codegen_reg(REG_RDI, SIZE_QWORD));
        codegen_emit_add(emitter, codegen_reg(REG_RAX, SIZE_QWORD), codegen_reg(REG_RSI, SIZE_QWORD));
    } else {
        // 32-bit, parameters on stack
        codegen_emit_prologue(emitter, 0);
        codegen_emit_mov(emitter, codegen_reg(REG_EAX, SIZE_DWORD), 
                         codegen_mem_base_disp(REG_EBP, 8, SIZE_DWORD));   // first parameter
        codegen_emit_add(emitter, codegen_reg(REG_EAX, SIZE_DWORD), 
                        codegen_mem_base_disp(REG_EBP, 12, SIZE_DWORD));  // second parameter
    }
    codegen_emit_epilogue(emitter);
    codegen_end_function(emitter);
    
    // create a simple main function that calls add_numbers
    codegen_emit_entry_point(emitter, "main");
    
    codegen_define_function(emitter, "main", true);
    codegen_emit_prologue(emitter, 0);
    
    // call add_numbers with arguments 10 and 20
    if (emitter->target.arch == ARCH_X86_64) {
        codegen_emit_mov(emitter, codegen_reg(REG_RDI, SIZE_QWORD), codegen_imm(10, SIZE_QWORD));
        codegen_emit_mov(emitter, codegen_reg(REG_RSI, SIZE_QWORD), codegen_imm(20, SIZE_QWORD));
        codegen_emit_call(emitter, "add_numbers");
    } else {
        // 32-bit, push parameters on stack
        codegen_emit_push(emitter, codegen_imm(20, SIZE_DWORD));
        codegen_emit_push(emitter, codegen_imm(10, SIZE_DWORD));
        codegen_emit_call(emitter, "add_numbers");
        codegen_emit_add(emitter, codegen_reg(REG_ESP, SIZE_DWORD), codegen_imm(8, SIZE_DWORD));  // clean stack
    }
    
    // return result to OS
    codegen_emit_epilogue(emitter);
    codegen_end_function(emitter);
    
    return true;
}
