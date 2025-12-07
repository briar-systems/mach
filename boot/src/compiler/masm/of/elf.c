#include "compiler/masm/of/elf.h"
#include "compiler/masm/isa/x86_64.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// elf64 type definitions
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

#define EI_NIDENT 16

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define PT_LOAD 1
#define PF_X 1
#define PF_W 2
#define PF_R 4

#define ET_EXEC 2
#define EM_X86_64 62

// Simple ELF writer for executable
int masm_elf_write(Masm *masm, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
    {
        return -1;
    }

    MasmSection *text   = masm_get_section(masm, ".text");
    MasmSection *rodata = masm_get_section(masm, ".rodata");
    MasmSection *data   = masm_get_section(masm, ".data");
    MasmSection *bss    = masm_get_section(masm, ".bss");

    if (!text)
    {
        fclose(f);
        return -1;
    }

    // pass 1: compute instruction sizes and symbol offsets consistently
    size_t current_offset = 0;
    uint64_t entry_offset = 0;

    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction *inst = &text->instructions[i];

        if (inst->opcode == MASM_OP_LABEL)
        {
            const char *lbl = inst->operands[0].label;
            if (lbl && strcmp(lbl, ".text") != 0)
            {
                MasmSymbol *sym = masm_get_symbol(masm, lbl);
                if (sym)
                {
                    sym->section_name = sym->section_name ? sym->section_name : strdup(text->name);
                    sym->offset       = current_offset;
                }
                if (strcmp(lbl, "_start") == 0)
                {
                    entry_offset = current_offset;
                }
            }
            continue;
        }

        size_t sz = 0;
        if (inst->opcode == MASM_OP_CALL && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            sz = 5;
        }
        else if (inst->opcode == MASM_OP_JMP && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            sz = 5;
        }
        else if (inst->opcode >= MASM_OP_JE && inst->opcode <= MASM_OP_JLE && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            sz = 6;
        }
        else if (inst->opcode == MASM_OP_MOV && inst->operands[1].kind == MASM_OPERAND_LABEL)
        {
            // encode with placeholder imm64
            MasmOperand     imm_op   = masm_operand_imm(0);
            MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst->operands[0], imm_op);
            sz = masm_x86_encode(tmp_inst, NULL, 0);
            masm_inst_destroy(tmp_inst);
        }
        else
        {
            sz = masm_x86_encode(*inst, NULL, 0);
        }

        current_offset += sz;
    }

    size_t text_size = current_offset;

    size_t rodata_size = rodata ? rodata->data_size : 0;
    size_t data_size   = data ? data->data_size : 0;
    size_t bss_size    = bss ? bss->data_size : 0;

    // base address constant for layout
    uint64_t base_addr   = 0x400000;

    size_t   buf_cap      = text_size > 0 ? text_size : 64;
    uint8_t *code_buffer  = malloc(buf_cap);
    size_t   code_size    = 0;
    int      label_errors = 0;

    // pass 2: encode with label resolution
    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction inst = text->instructions[i];
        if (inst.opcode == MASM_OP_LABEL)
        {
            continue;
        }

        if (inst.opcode == MASM_OP_CALL && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 5);
                if (rel64 < INT32_MIN || rel64 > INT32_MAX)
                {
                    fprintf(stderr, "error: call to '%s' exceeds 32-bit relative offset (distance: %ld)\n", inst.operands[0].label, rel64);
                    label_errors++;
                }
                int32_t rel = (int32_t)rel64;

                code_buffer[code_size++] = 0xE8;

                for (int k = 0; k < 4; k++)
                {
                    code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                }
                continue;
            }
            else
            {
                fprintf(stderr, "warning: undefined symbol '%s' (skipping call)\n", inst.operands[0].label);
                continue;
            }
        }
        else if (inst.opcode == MASM_OP_JMP && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 5);
                if (rel64 < INT32_MIN || rel64 > INT32_MAX)
                {
                    fprintf(stderr, "error: jump to '%s' exceeds 32-bit relative offset (distance: %ld)\n", inst.operands[0].label, rel64);
                    label_errors++;
                }
                int32_t rel = (int32_t)rel64;

                code_buffer[code_size++] = 0xE9;

                for (int k = 0; k < 4; k++)
                {
                    code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                }
                continue;
            }
            else
            {
                fprintf(stderr, "warning: undefined symbol '%s' (skipping jmp)\n", inst.operands[0].label);
                continue;
            }
        }
        else if (inst.opcode >= MASM_OP_JE && inst.opcode <= MASM_OP_JLE && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 6);
                if (rel64 < INT32_MIN || rel64 > INT32_MAX)
                {
                    fprintf(stderr, "error: conditional jump to '%s' exceeds 32-bit relative offset (distance: %ld)\n", inst.operands[0].label, rel64);
                    label_errors++;
                }
                int32_t rel = (int32_t)rel64;

                code_buffer[code_size++] = 0x0F;

                uint8_t opcode = 0x84;
                switch (inst.opcode)
                {
                case MASM_OP_JE:
                    opcode = 0x84;
                    break;
                case MASM_OP_JNE:
                    opcode = 0x85;
                    break;
                case MASM_OP_JL:
                    opcode = 0x8C;
                    break;
                case MASM_OP_JG:
                    opcode = 0x8F;
                    break;
                case MASM_OP_JLE:
                    opcode = 0x8E;
                    break;
                case MASM_OP_JGE:
                    opcode = 0x8D;
                    break;
                }
                code_buffer[code_size++] = opcode;

                for (int k = 0; k < 4; k++)
                {
                    code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                }
                continue;
            }
            else
            {
                fprintf(stderr, "warning: undefined symbol '%s' (skipping conditional jump)\n", inst.operands[0].label);
                continue;
            }
        }
        else if (inst.opcode == MASM_OP_MOV && inst.operands[1].kind == MASM_OPERAND_LABEL)
        {
            // MOV reg, label -> MOV reg, imm64 (absolute address)
            // first pass: just account for size using a placeholder immediate
            MasmOperand     imm_op   = masm_operand_imm(0);
            MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst.operands[0], imm_op);
            code_size += masm_x86_encode(tmp_inst, code_buffer + code_size, buf_cap - code_size);
            masm_inst_destroy(tmp_inst);
            continue;
        }

        // ensure capacity (worst-case instruction length ~15 bytes)
        if (code_size + 16 > buf_cap)
        {
            buf_cap *= 2;
            code_buffer = realloc(code_buffer, buf_cap);
        }

        code_size += masm_x86_encode(inst, code_buffer + code_size, buf_cap - code_size);
    }

    // use the actual encoded size for text size
    text_size = code_size;

    // recompute accurate entry_offset using final instruction sizes
    size_t recomputed_offset = 0;
    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction inst = text->instructions[i];
        if (inst.opcode == MASM_OP_LABEL)
        {
            if (inst.operands[0].label && strcmp(inst.operands[0].label, "_start") == 0)
            {
                entry_offset = recomputed_offset;
                break;
            }
            continue;
        }

        if (inst.opcode == MASM_OP_CALL && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            recomputed_offset += 5;
        }
        else if (inst.opcode == MASM_OP_JMP && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            recomputed_offset += 5;
        }
        else if (inst.opcode >= MASM_OP_JE && inst.opcode <= MASM_OP_JLE && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            recomputed_offset += 6;
        }
        else if (inst.opcode == MASM_OP_MOV && inst.operands[1].kind == MASM_OPERAND_LABEL)
        {
            MasmOperand     imm_op   = masm_operand_imm(0);
            MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst.operands[0], imm_op);
            recomputed_offset += masm_x86_encode(tmp_inst, NULL, 0);
            masm_inst_destroy(tmp_inst);
        }
        else
        {
            recomputed_offset += masm_x86_encode(inst, NULL, 0);
        }
    }

    // layout now that text_size is known (will be recomputed after final encode as well)
    size_t rodata_start_offset = text_size;
    uint64_t seg1_offset       = 0x1000;
    uint64_t seg1_vaddr        = base_addr + seg1_offset;
    size_t   seg1_filesz       = text_size;
    size_t   seg1_memsz        = text_size;

    if (rodata_size > 0)
    {
        size_t pad = (16 - (seg1_filesz % 16)) % 16;
        seg1_filesz += pad;
        seg1_memsz += pad;
        rodata_start_offset = seg1_filesz;
        seg1_filesz += rodata_size;
        seg1_memsz += rodata_size;
    }

    uint64_t seg2_offset = (seg1_offset + seg1_filesz + 0xFFF) & ~0xFFF;
    uint64_t seg2_vaddr  = base_addr + seg2_offset;
    size_t   seg2_filesz = 0;
    size_t   seg2_memsz  = 0;
    if (data_size > 0 || bss_size > 0)
    {
        seg2_filesz = data_size + bss_size;
        seg2_memsz  = seg2_filesz;
    }

    int phnum = 1 + (seg2_memsz > 0 ? 1 : 0);

    // re-encode with correct layout for label-based MOV (rodata/data addresses)
    code_size = 0;
    if (buf_cap < text_size)
    {
        buf_cap = text_size;
        code_buffer = realloc(code_buffer, buf_cap);
    }

    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction inst = text->instructions[i];
        if (inst.opcode == MASM_OP_LABEL)
        {
            continue;
        }

        if (inst.opcode == MASM_OP_CALL && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 5);
                int32_t rel   = (int32_t)rel64;
                code_buffer[code_size++] = 0xE8;
                for (int k = 0; k < 4; k++) code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                continue;
            }
            else
            {
                label_errors++;
                continue;
            }
        }
        else if (inst.opcode == MASM_OP_JMP && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 5);
                int32_t rel   = (int32_t)rel64;
                code_buffer[code_size++] = 0xE9;
                for (int k = 0; k < 4; k++) code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                continue;
            }
            else
            {
                label_errors++;
                continue;
            }
        }
        else if (inst.opcode >= MASM_OP_JE && inst.opcode <= MASM_OP_JLE && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                int64_t rel64 = (int64_t)target->offset - (int64_t)(code_size + 6);
                int32_t rel   = (int32_t)rel64;
                code_buffer[code_size++] = 0x0F;
                uint8_t opcode = 0x84;
                switch (inst.opcode)
                {
                case MASM_OP_JNE: opcode = 0x85; break;
                case MASM_OP_JL: opcode = 0x8C; break;
                case MASM_OP_JG: opcode = 0x8F; break;
                case MASM_OP_JLE: opcode = 0x8E; break;
                case MASM_OP_JGE: opcode = 0x8D; break;
                default: opcode = 0x84; break;
                }
                code_buffer[code_size++] = opcode;
                for (int k = 0; k < 4; k++) code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                continue;
            }
            else
            {
                label_errors++;
                continue;
            }
        }
        else if (inst.opcode == MASM_OP_MOV && inst.operands[1].kind == MASM_OPERAND_LABEL)
        {
            MasmSymbol *sym = masm_get_symbol(masm, inst.operands[1].label);
            if (sym)
            {
                uint64_t addr = 0;
                if (sym->section_name && strcmp(sym->section_name, ".rodata") == 0)
                {
                    addr = seg1_vaddr + rodata_start_offset + sym->offset;
                }
                else if (sym->section_name && strcmp(sym->section_name, ".data") == 0)
                {
                    addr = seg2_vaddr + sym->offset;
                }
                else if (sym->section_name && strcmp(sym->section_name, ".bss") == 0)
                {
                    addr = seg2_vaddr + data_size + sym->offset;
                }
                else if (sym->section_name && strcmp(sym->section_name, ".text") == 0)
                {
                    addr = seg1_vaddr + sym->offset;
                }

                MasmOperand     imm_op   = masm_operand_imm(addr);
                MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst.operands[0], imm_op);
                code_size += masm_x86_encode(tmp_inst, code_buffer + code_size, buf_cap - code_size);
                masm_inst_destroy(tmp_inst);
                continue;
            }
            else
            {
                label_errors++;
                continue;
            }
        }

        if (code_size + 16 > buf_cap)
        {
            buf_cap *= 2;
            code_buffer = realloc(code_buffer, buf_cap);
        }
        code_size += masm_x86_encode(inst, code_buffer + code_size, buf_cap - code_size);
    }

    text_size = code_size;

    // recompute layout with final text_size so program header sizes match written data
    rodata_start_offset = text_size;
    seg1_filesz         = text_size;
    seg1_memsz          = text_size;

    if (rodata_size > 0)
    {
        size_t pad = (16 - (seg1_filesz % 16)) % 16;
        seg1_filesz += pad;
        seg1_memsz += pad;
        rodata_start_offset = seg1_filesz;
        seg1_filesz += rodata_size;
        seg1_memsz += rodata_size;
    }

    // ensure the load segment covers everything we will write (text + rodata)
    seg1_filesz = rodata_start_offset + (rodata_size > 0 ? rodata_size : 0);
    seg1_memsz  = seg1_filesz;

    seg2_offset = (seg1_offset + seg1_filesz + 0xFFF) & ~0xFFF;
    seg2_vaddr  = base_addr + seg2_offset;
    seg2_filesz = 0;
    seg2_memsz  = 0;
    if (data_size > 0 || bss_size > 0)
    {
        seg2_filesz = data_size + bss_size;
        seg2_memsz  = seg2_filesz;
    }
    phnum = 1 + (seg2_memsz > 0 ? 1 : 0);

    if (label_errors > 0)
    {
        fprintf(stderr, "error: %d label resolution error(s)\n", label_errors);
        free(code_buffer);
        fclose(f);
        return -1;
    }

    // rewind and write ELF headers with final sizes
    fseek(f, 0, SEEK_SET);

    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    ehdr.e_ident[0]  = 0x7F;
    ehdr.e_ident[1]  = 'E';
    ehdr.e_ident[2]  = 'L';
    ehdr.e_ident[3]  = 'F';
    ehdr.e_ident[4]  = 2;
    ehdr.e_ident[5]  = 1;
    ehdr.e_ident[6]  = 1;
    ehdr.e_ident[7]  = 0;
    ehdr.e_type      = ET_EXEC;
    ehdr.e_machine   = EM_X86_64;
    ehdr.e_version   = 1;
    ehdr.e_entry     = seg1_vaddr + entry_offset;
    ehdr.e_phoff     = sizeof(Elf64_Ehdr);
    ehdr.e_shoff     = 0;
    ehdr.e_ehsize    = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum     = phnum;
    ehdr.e_shentsize = 0;
    ehdr.e_shnum     = 0;
    ehdr.e_shstrndx  = 0;
    fwrite(&ehdr, 1, sizeof(ehdr), f);

    Elf64_Phdr phdr;
    memset(&phdr, 0, sizeof(phdr));
    phdr.p_type   = PT_LOAD;
    phdr.p_flags  = PF_R | PF_X;
    phdr.p_offset = seg1_offset;
    phdr.p_vaddr  = seg1_vaddr;
    phdr.p_paddr  = seg1_vaddr;
    phdr.p_filesz = seg1_filesz;
    phdr.p_memsz  = seg1_memsz;
    phdr.p_align  = 0x1000;
    fwrite(&phdr, 1, sizeof(phdr), f);

    if (phnum > 1)
    {
        Elf64_Phdr phdr2;
        memset(&phdr2, 0, sizeof(phdr2));
        phdr2.p_type   = PT_LOAD;
        phdr2.p_flags  = PF_R | PF_W;
        phdr2.p_offset = seg2_offset;
        phdr2.p_vaddr  = seg2_vaddr;
        phdr2.p_paddr  = seg2_vaddr;
        phdr2.p_filesz = seg2_filesz;
        phdr2.p_memsz  = seg2_memsz;
        phdr2.p_align  = 0x1000;
        fwrite(&phdr2, 1, sizeof(phdr2), f);
    }

    fseek(f, seg1_offset, SEEK_SET);
    fwrite(code_buffer, 1, code_size, f);

    size_t padding = rodata_start_offset > text_size ? rodata_start_offset - text_size : 0;
    if (padding > 0)
    {
        uint8_t *pad = calloc(1, padding);
        fwrite(pad, 1, padding, f);
        free(pad);
    }

    if (rodata_size > 0)
    {
        fwrite(rodata->data, 1, rodata_size, f);
    }

    if (phnum > 1)
    {
        fseek(f, seg2_offset, SEEK_SET);
        if (data_size > 0) fwrite(data->data, 1, data_size, f);
        if (bss_size > 0) fwrite(bss->data, 1, bss_size, f);
    }

    free(code_buffer);
    fclose(f);
    return 0;
}
