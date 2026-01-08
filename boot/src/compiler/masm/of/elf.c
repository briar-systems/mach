// elf object writer
//
// the bootstrap compiler used to write a minimalist ET_EXEC binary directly and
// do ad-hoc label resolution (including skipping undefined calls).
//
// this file now emits a more normal ET_REL relocatable object:
// - proper section headers (.text/.rodata/.data/.bss)
// - symbol table (.symtab/.strtab)
// - relocations (.rela.text)
//
// final linking/archiving is handled by the driver (cmd_build) using external
// tools (cc/ld/ar), similar to a traditional toolchain.

#include "compiler/masm/of/elf.h"
#include "compiler/masm/isa/x86_64.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
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
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct
{
    Elf64_Word  st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half  st_shndx;
    Elf64_Addr  st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

typedef struct
{
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

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

#define ET_REL 1
#define EM_X86_64 62

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS64 2
#define ELFDATA2LSB 1

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_NOBITS 8

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3

#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

#define R_X86_64_64 1
#define R_X86_64_PC32 2

#define ELF64_R_INFO(sym, type) ((((Elf64_Xword)(sym)) << 32) | ((type) & 0xffffffff))

typedef struct
{
    char  *data;
    size_t len;
    size_t cap;
} StrTab;

static void strtab_init(StrTab *t)
{
    memset(t, 0, sizeof(*t));
    t->cap  = 64;
    t->data = malloc(t->cap);
    t->len  = 0;
    // first entry is empty string
    t->data[t->len++] = '\0';
}

static void strtab_dnit(StrTab *t)
{
    free(t->data);
    t->data = NULL;
    t->len  = 0;
    t->cap  = 0;
}

static Elf64_Word strtab_add(StrTab *t, const char *s)
{
    if (!s || !*s)
    {
        return 0;
    }

    size_t n = strlen(s) + 1;
    if (t->len + n > t->cap)
    {
        while (t->len + n > t->cap)
        {
            t->cap *= 2;
        }
        t->data = realloc(t->data, t->cap);
    }

    Elf64_Word off = (Elf64_Word)t->len;
    memcpy(t->data + t->len, s, n);
    t->len += n;
    return off;
}

typedef struct
{
    char      *name;
    Elf64_Word sym_index;
} SymIndex;

static Elf64_Word section_index_for(const char *section_name)
{
    if (!section_name)
    {
        return 0;
    }
    if (strcmp(section_name, ".text") == 0)
    {
        return 1;
    }
    if (strcmp(section_name, ".rodata") == 0)
    {
        return 2;
    }
    if (strcmp(section_name, ".data") == 0)
    {
        return 3;
    }
    if (strcmp(section_name, ".bss") == 0)
    {
        return 4;
    }
    return 0;
}

static uint8_t bind_to_elf(MasmSymbolBind b)
{
    switch (b)
    {
    case MASM_BIND_LOCAL:
        return STB_LOCAL;
    case MASM_BIND_GLOBAL:
        return STB_GLOBAL;
    case MASM_BIND_WEAK:
        return STB_WEAK;
    default:
        return STB_LOCAL;
    }
}

static uint8_t kind_to_elf_type(MasmSymbolKind k)
{
    switch (k)
    {
    case MASM_SYMBOL_FUNCTION:
        return STT_FUNC;
    case MASM_SYMBOL_DATA:
        return STT_OBJECT;
    case MASM_SYMBOL_LABEL:
    default:
        return STT_NOTYPE;
    }
}

static Elf64_Word sym_lookup_or_add(StrTab *strtab, Elf64_Sym **syms, size_t *sym_count, size_t *sym_cap,
                                    SymIndex **idx, size_t *idx_count, size_t *idx_cap,
                                    const char *name)
{
    for (size_t i = 0; i < *idx_count; i++)
    {
        if (strcmp((*idx)[i].name, name) == 0)
        {
            return (*idx)[i].sym_index;
        }
    }

    if (*sym_count + 1 > *sym_cap)
    {
        *sym_cap = (*sym_cap == 0) ? 64 : (*sym_cap * 2);
        *syms    = realloc(*syms, sizeof(Elf64_Sym) * (*sym_cap));
    }

    if (*idx_count + 1 > *idx_cap)
    {
        *idx_cap = (*idx_cap == 0) ? 64 : (*idx_cap * 2);
        *idx     = realloc(*idx, sizeof(SymIndex) * (*idx_cap));
    }

    Elf64_Sym s;
    memset(&s, 0, sizeof(s));
    s.st_name  = strtab_add(strtab, name);
    s.st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE);
    s.st_other = 0;
    s.st_shndx = 0; // undefined
    s.st_value = 0;
    s.st_size  = 0;

    Elf64_Word new_idx = (Elf64_Word)(*sym_count);
    (*syms)[(*sym_count)++] = s;

    (*idx)[*idx_count].name      = strdup(name);
    (*idx)[*idx_count].sym_index = new_idx;
    (*idx_count)++;

    return new_idx;
}

static size_t align_up(size_t n, size_t a)
{
    if (a == 0)
    {
        return n;
    }
    size_t r = n % a;
    return r ? (n + (a - r)) : n;
}

// write an ELF64 relocatable object (ET_REL)
int masm_elf_write(Masm *masm, const char *filename)
{
    if (!masm || !filename)
    {
        return -1;
    }

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

    // pass 1: compute text offsets and attach symbol offsets for labels.
    size_t text_off = 0;
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
                    if (!sym->section_name)
                    {
                        sym->section_name = strdup(".text");
                    }
                    sym->offset = text_off;
                }
            }
            continue;
        }

        if (inst->opcode == MASM_OP_CALL && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            text_off += 5;
        }
        else if (inst->opcode == MASM_OP_JMP && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            text_off += 5;
        }
        else if (inst->opcode >= MASM_OP_JE && inst->opcode <= MASM_OP_JLE && inst->operands[0].kind == MASM_OPERAND_LABEL)
        {
            text_off += 6;
        }
        else if (inst->opcode == MASM_OP_MOV && inst->operand_count == 2 && inst->operands[0].kind == MASM_OPERAND_REGISTER && inst->operands[1].kind == MASM_OPERAND_LABEL)
        {
            // mov r64, imm64 (rex + opcode + imm)
            uint32_t dst = inst->operands[0].reg.id;
            uint8_t  sz  = inst->operands[0].reg.size;

            if (sz == 8)
            {
                text_off += (dst >= 8) ? 10 : 10;
            }
            else
            {
                MasmOperand     imm_op   = masm_operand_imm(0);
                MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst->operands[0], imm_op);
                text_off += masm_x86_encode(tmp_inst, NULL, 0);
                masm_inst_destroy(tmp_inst);
            }
        }
        else
        {
            text_off += masm_x86_encode(*inst, NULL, 0);
        }
    }

    size_t text_size   = text_off;
    size_t rodata_size = rodata ? rodata->data_size : 0;
    size_t data_size   = data ? data->data_size : 0;
    size_t bss_size    = bss ? bss->data_size : 0;

    // build strtabs
    StrTab shstr;
    StrTab str;
    strtab_init(&shstr);
    strtab_init(&str);

    Elf64_Word sh_null    = 0;
    Elf64_Word sh_text    = strtab_add(&shstr, ".text");
    Elf64_Word sh_rodata  = strtab_add(&shstr, ".rodata");
    Elf64_Word sh_data    = strtab_add(&shstr, ".data");
    Elf64_Word sh_bss     = strtab_add(&shstr, ".bss");
    Elf64_Word sh_relatex = strtab_add(&shstr, ".rela.text");
    Elf64_Word sh_symtab  = strtab_add(&shstr, ".symtab");
    Elf64_Word sh_strtab  = strtab_add(&shstr, ".strtab");
    Elf64_Word sh_shstr   = strtab_add(&shstr, ".shstrtab");

    // build symbol table
    Elf64_Sym *syms      = NULL;
    size_t     sym_count = 0;
    size_t     sym_cap   = 0;

    SymIndex *sym_index = NULL;
    size_t    idx_count = 0;
    size_t    idx_cap   = 0;

    // undef symbol
    Elf64_Sym undef;
    memset(&undef, 0, sizeof(undef));
    syms    = malloc(sizeof(Elf64_Sym) * 64);
    sym_cap = 64;
    syms[sym_count++] = undef;

    // section symbols (local)
    Elf64_Sym sec;
    memset(&sec, 0, sizeof(sec));
    sec.st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION);
    sec.st_shndx = 1;
    syms[sym_count++] = sec;
    sec.st_shndx = 2;
    syms[sym_count++] = sec;
    sec.st_shndx = 3;
    syms[sym_count++] = sec;
    sec.st_shndx = 4;
    syms[sym_count++] = sec;

    // add defined masm symbols (locals first, then globals/weak)
    for (size_t pass = 0; pass < 2; pass++)
    {
        for (size_t i = 0; i < masm->symbol_count; i++)
        {
            MasmSymbol *ms = masm->symbols[i];
            if (!ms || !ms->name)
            {
                continue;
            }

            bool is_local = ms->bind == MASM_BIND_LOCAL;
            if ((pass == 0) != is_local)
            {
                continue;
            }

            Elf64_Sym s;
            memset(&s, 0, sizeof(s));
            s.st_name  = strtab_add(&str, ms->name);
            s.st_info  = ELF64_ST_INFO(bind_to_elf(ms->bind), kind_to_elf_type(ms->kind));
            s.st_other = 0;
            s.st_shndx = section_index_for(ms->section_name);
            s.st_value = ms->offset;
            s.st_size  = ms->size;

            if (sym_count >= sym_cap)
            {
                sym_cap *= 2;
                syms = realloc(syms, sizeof(Elf64_Sym) * sym_cap);
            }
            Elf64_Word si = (Elf64_Word)sym_count;
            syms[sym_count++] = s;

            if (idx_count >= idx_cap)
            {
                idx_cap = idx_cap == 0 ? 64 : idx_cap * 2;
                sym_index = realloc(sym_index, sizeof(SymIndex) * idx_cap);
            }
            sym_index[idx_count].name      = strdup(ms->name);
            sym_index[idx_count].sym_index = si;
            idx_count++;
        }
    }

    // record local symbol cutoff for symtab sh_info
    // section symbols are local; all masm locals were appended in the first pass.
    Elf64_Word local_count = 0;
    for (size_t i = 0; i < sym_count; i++)
    {
        uint8_t bind = syms[i].st_info >> 4;
        if (bind != STB_LOCAL)
        {
            local_count = (Elf64_Word)i;
            break;
        }
        local_count = (Elf64_Word)(i + 1);
    }

    // relocation list
    Elf64_Rela *rela       = NULL;
    size_t      rela_count = 0;
    size_t      rela_cap   = 0;

    // encode .text and collect relocations
    uint8_t *text_buf = malloc(text_size ? text_size : 1);
    size_t   off = 0;

    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction inst = text->instructions[i];
        if (inst.opcode == MASM_OP_LABEL)
        {
            continue;
        }

        if ((inst.opcode == MASM_OP_CALL || inst.opcode == MASM_OP_JMP) && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            const char *name = inst.operands[0].label;
            Elf64_Word  si   = sym_lookup_or_add(&str, &syms, &sym_count, &sym_cap, &sym_index, &idx_count, &idx_cap, name);

            text_buf[off++] = (inst.opcode == MASM_OP_CALL) ? 0xE8 : 0xE9;
            Elf64_Addr rel_off = (Elf64_Addr)off;
            text_buf[off++] = 0;
            text_buf[off++] = 0;
            text_buf[off++] = 0;
            text_buf[off++] = 0;

            if (rela_count >= rela_cap)
            {
                rela_cap = rela_cap == 0 ? 64 : rela_cap * 2;
                rela = realloc(rela, sizeof(Elf64_Rela) * rela_cap);
            }
            rela[rela_count].r_offset = rel_off;
            rela[rela_count].r_info   = ELF64_R_INFO(si, R_X86_64_PC32);
            rela[rela_count].r_addend = -4;
            rela_count++;
            continue;
        }

        if (inst.opcode >= MASM_OP_JE && inst.opcode <= MASM_OP_JLE && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            const char *name = inst.operands[0].label;
            Elf64_Word  si   = sym_lookup_or_add(&str, &syms, &sym_count, &sym_cap, &sym_index, &idx_count, &idx_cap, name);

            uint8_t cond = 0x84;
            switch (inst.opcode)
            {
            case MASM_OP_JE:
                cond = 0x84;
                break;
            case MASM_OP_JNE:
                cond = 0x85;
                break;
            case MASM_OP_JL:
                cond = 0x8C;
                break;
            case MASM_OP_JLE:
                cond = 0x8E;
                break;
            case MASM_OP_JG:
                cond = 0x8F;
                break;
            case MASM_OP_JGE:
                cond = 0x8D;
                break;
            default:
                cond = 0x84;
                break;
            }

            text_buf[off++] = 0x0F;
            text_buf[off++] = cond;
            Elf64_Addr rel_off = (Elf64_Addr)off;
            text_buf[off++] = 0;
            text_buf[off++] = 0;
            text_buf[off++] = 0;
            text_buf[off++] = 0;

            if (rela_count >= rela_cap)
            {
                rela_cap = rela_cap == 0 ? 64 : rela_cap * 2;
                rela = realloc(rela, sizeof(Elf64_Rela) * rela_cap);
            }
            rela[rela_count].r_offset = rel_off;
            rela[rela_count].r_info   = ELF64_R_INFO(si, R_X86_64_PC32);
            rela[rela_count].r_addend = -4;
            rela_count++;
            continue;
        }

        if (inst.opcode == MASM_OP_MOV && inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_LABEL)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint8_t  sz  = inst.operands[0].reg.size;
            if (sz != 8)
            {
                MasmOperand     imm_op   = masm_operand_imm(0);
                MasmInstruction tmp_inst = masm_inst_2(MASM_OP_MOV, inst.operands[0], imm_op);
                off += masm_x86_encode(tmp_inst, text_buf + off, text_size - off);
                masm_inst_destroy(tmp_inst);
                continue;
            }

            const char *name = inst.operands[1].label;
            Elf64_Word  si   = sym_lookup_or_add(&str, &syms, &sym_count, &sym_cap, &sym_index, &idx_count, &idx_cap, name);

            // rex.w + mov r64, imm64
            text_buf[off++] = (dst >= 8) ? 0x49 : 0x48;
            text_buf[off++] = (uint8_t)(0xB8 + (dst & 7));

            Elf64_Addr rel_off = (Elf64_Addr)off;
            for (int k = 0; k < 8; k++)
            {
                text_buf[off++] = 0;
            }

            if (rela_count >= rela_cap)
            {
                rela_cap = rela_cap == 0 ? 64 : rela_cap * 2;
                rela = realloc(rela, sizeof(Elf64_Rela) * rela_cap);
            }
            rela[rela_count].r_offset = rel_off;
            rela[rela_count].r_info   = ELF64_R_INFO(si, R_X86_64_64);
            rela[rela_count].r_addend = 0;
            rela_count++;
            continue;
        }

        off += masm_x86_encode(inst, text_buf + off, text_size - off);
    }

    if (off != text_size)
    {
        text_size = off;
    }

    // compute file layout
    size_t file_off = sizeof(Elf64_Ehdr);

    size_t text_file_off = align_up(file_off, 16);
    file_off = text_file_off + text_size;

    size_t rodata_file_off = align_up(file_off, 16);
    file_off = rodata_file_off + rodata_size;

    size_t data_file_off = align_up(file_off, 16);
    file_off = data_file_off + data_size;

    size_t rela_file_off = align_up(file_off, 8);
    size_t rela_size = rela_count * sizeof(Elf64_Rela);
    file_off = rela_file_off + rela_size;

    size_t symtab_file_off = align_up(file_off, 8);
    size_t symtab_size = sym_count * sizeof(Elf64_Sym);
    file_off = symtab_file_off + symtab_size;

    size_t strtab_file_off = align_up(file_off, 1);
    size_t strtab_size = str.len;
    file_off = strtab_file_off + strtab_size;

    size_t shstr_file_off = align_up(file_off, 1);
    size_t shstr_size = shstr.len;
    file_off = shstr_file_off + shstr_size;

    size_t shoff = align_up(file_off, 8);

    const int shnum = 9;
    Elf64_Shdr shdrs[shnum];
    memset(shdrs, 0, sizeof(shdrs));

    shdrs[0].sh_name = sh_null;
    shdrs[0].sh_type = SHT_NULL;

    shdrs[1].sh_name      = sh_text;
    shdrs[1].sh_type      = SHT_PROGBITS;
    shdrs[1].sh_flags     = SHF_ALLOC | SHF_EXECINSTR;
    shdrs[1].sh_offset    = text_file_off;
    shdrs[1].sh_size      = text_size;
    shdrs[1].sh_addralign = 16;

    shdrs[2].sh_name      = sh_rodata;
    shdrs[2].sh_type      = SHT_PROGBITS;
    shdrs[2].sh_flags     = SHF_ALLOC;
    shdrs[2].sh_offset    = rodata_file_off;
    shdrs[2].sh_size      = rodata_size;
    shdrs[2].sh_addralign = 16;

    shdrs[3].sh_name      = sh_data;
    shdrs[3].sh_type      = SHT_PROGBITS;
    shdrs[3].sh_flags     = SHF_ALLOC | SHF_WRITE;
    shdrs[3].sh_offset    = data_file_off;
    shdrs[3].sh_size      = data_size;
    shdrs[3].sh_addralign = 16;

    shdrs[4].sh_name      = sh_bss;
    shdrs[4].sh_type      = SHT_NOBITS;
    shdrs[4].sh_flags     = SHF_ALLOC | SHF_WRITE;
    shdrs[4].sh_offset    = 0;
    shdrs[4].sh_size      = bss_size;
    shdrs[4].sh_addralign = 16;

    shdrs[5].sh_name      = sh_relatex;
    shdrs[5].sh_type      = SHT_RELA;
    shdrs[5].sh_flags     = 0;
    shdrs[5].sh_offset    = rela_file_off;
    shdrs[5].sh_size      = rela_size;
    shdrs[5].sh_link      = 6;
    shdrs[5].sh_info      = 1;
    shdrs[5].sh_addralign = 8;
    shdrs[5].sh_entsize   = sizeof(Elf64_Rela);

    shdrs[6].sh_name      = sh_symtab;
    shdrs[6].sh_type      = SHT_SYMTAB;
    shdrs[6].sh_flags     = 0;
    shdrs[6].sh_offset    = symtab_file_off;
    shdrs[6].sh_size      = symtab_size;
    shdrs[6].sh_link      = 7;
    shdrs[6].sh_info      = local_count;
    shdrs[6].sh_addralign = 8;
    shdrs[6].sh_entsize   = sizeof(Elf64_Sym);

    shdrs[7].sh_name      = sh_strtab;
    shdrs[7].sh_type      = SHT_STRTAB;
    shdrs[7].sh_flags     = 0;
    shdrs[7].sh_offset    = strtab_file_off;
    shdrs[7].sh_size      = strtab_size;
    shdrs[7].sh_addralign = 1;

    shdrs[8].sh_name      = sh_shstr;
    shdrs[8].sh_type      = SHT_STRTAB;
    shdrs[8].sh_flags     = 0;
    shdrs[8].sh_offset    = shstr_file_off;
    shdrs[8].sh_size      = shstr_size;
    shdrs[8].sh_addralign = 1;

    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    ehdr.e_ident[EI_MAG0]    = ELFMAG0;
    ehdr.e_ident[EI_MAG1]    = ELFMAG1;
    ehdr.e_ident[EI_MAG2]    = ELFMAG2;
    ehdr.e_ident[EI_MAG3]    = ELFMAG3;
    ehdr.e_ident[EI_CLASS]   = ELFCLASS64;
    ehdr.e_ident[EI_DATA]    = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = 1;
    ehdr.e_type              = ET_REL;
    ehdr.e_machine           = EM_X86_64;
    ehdr.e_version           = 1;
    ehdr.e_ehsize            = sizeof(Elf64_Ehdr);
    ehdr.e_shoff             = (Elf64_Off)shoff;
    ehdr.e_shentsize         = sizeof(Elf64_Shdr);
    ehdr.e_shnum             = shnum;
    ehdr.e_shstrndx          = 8;

    fwrite(&ehdr, 1, sizeof(ehdr), f);

    long pos = ftell(f);
    while ((size_t)pos < text_file_off)
    {
        fputc(0, f);
        pos++;
    }
    fwrite(text_buf, 1, text_size, f);

    pos = ftell(f);
    while ((size_t)pos < rodata_file_off)
    {
        fputc(0, f);
        pos++;
    }
    if (rodata_size > 0)
    {
        fwrite(rodata->data, 1, rodata_size, f);
    }

    pos = ftell(f);
    while ((size_t)pos < data_file_off)
    {
        fputc(0, f);
        pos++;
    }
    if (data_size > 0)
    {
        fwrite(data->data, 1, data_size, f);
    }

    pos = ftell(f);
    while ((size_t)pos < rela_file_off)
    {
        fputc(0, f);
        pos++;
    }
    if (rela_size > 0)
    {
        fwrite(rela, 1, rela_size, f);
    }

    pos = ftell(f);
    while ((size_t)pos < symtab_file_off)
    {
        fputc(0, f);
        pos++;
    }
    fwrite(syms, 1, symtab_size, f);

    pos = ftell(f);
    while ((size_t)pos < strtab_file_off)
    {
        fputc(0, f);
        pos++;
    }
    fwrite(str.data, 1, strtab_size, f);

    pos = ftell(f);
    while ((size_t)pos < shstr_file_off)
    {
        fputc(0, f);
        pos++;
    }
    fwrite(shstr.data, 1, shstr_size, f);

    pos = ftell(f);
    while ((size_t)pos < shoff)
    {
        fputc(0, f);
        pos++;
    }
    fwrite(shdrs, 1, sizeof(shdrs), f);

    fclose(f);

    free(text_buf);
    free(rela);
    for (size_t i = 0; i < idx_count; i++)
    {
        free(sym_index[i].name);
    }
    free(sym_index);
    free(syms);
    strtab_dnit(&str);
    strtab_dnit(&shstr);

    return 0;
}
