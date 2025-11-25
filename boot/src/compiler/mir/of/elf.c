#include "compiler/mir/of/elf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// elf64 header
typedef struct
{
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

// elf64 section header
typedef struct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} Elf64_Shdr;

// elf64 symbol
typedef struct
{
    uint32_t st_name;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

// elf64 relocation with addend
typedef struct
{
    uint64_t r_offset;
    uint64_t r_info;
    int64_t  r_addend;
} Elf64_Rela;

// section info
typedef struct ELFSection
{
    char            *name;
    ELFSectionType   type;
    uint32_t         flags;
    uint8_t         *data;
    size_t           size;
    size_t           capacity;
    struct ELFSection *next;
} ELFSection;

// symbol info
typedef struct ELFSymbol
{
    char         *name;
    uint64_t      value;
    int           section_id;
    bool          is_global;
    bool          is_function;
    struct ELFSymbol *next;
} ELFSymbol;

// relocation info
typedef struct ELFRelocation
{
    int                section_id;
    uint64_t           offset;
    char              *symbol_name;
    ELFX86_64RelocType type;
    struct ELFRelocation *next;
} ELFRelocation;

// elf context
struct ELFContext
{
    char           *filename;
    ELFSection     *sections;
    ELFSymbol      *symbols;
    ELFRelocation  *relocations;
    int             section_count;
    int             symbol_count;
    int             reloc_count;
};

ELFContext *elf_context_create(const char *filename)
{
    ELFContext *ctx = malloc(sizeof(ELFContext));
    if (!ctx)
    {
        return NULL;
    }

    ctx->filename = filename ? strdup(filename) : NULL;
    ctx->sections = NULL;
    ctx->symbols = NULL;
    ctx->relocations = NULL;
    ctx->section_count = 0;
    ctx->symbol_count = 0;
    ctx->reloc_count = 0;

    return ctx;
}

void elf_context_destroy(ELFContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->filename)
    {
        free(ctx->filename);
    }

    // free sections
    ELFSection *sec = ctx->sections;
    while (sec)
    {
        ELFSection *next = sec->next;
        if (sec->name)
        {
            free(sec->name);
        }
        if (sec->data)
        {
            free(sec->data);
        }
        free(sec);
        sec = next;
    }

    // free symbols
    ELFSymbol *sym = ctx->symbols;
    while (sym)
    {
        ELFSymbol *next = sym->next;
        if (sym->name)
        {
            free(sym->name);
        }
        free(sym);
        sym = next;
    }

    // free relocations
    ELFRelocation *rel = ctx->relocations;
    while (rel)
    {
        ELFRelocation *next = rel->next;
        if (rel->symbol_name)
        {
            free(rel->symbol_name);
        }
        free(rel);
        rel = next;
    }

    free(ctx);
}

int elf_create_section(ELFContext *ctx, const char *name, ELFSectionType type, uint32_t flags)
{
    if (!ctx || !name)
    {
        return -1;
    }

    ELFSection *section = malloc(sizeof(ELFSection));
    if (!section)
    {
        return -1;
    }

    section->name = strdup(name);
    section->type = type;
    section->flags = flags;
    section->data = NULL;
    section->size = 0;
    section->capacity = 0;
    section->next = ctx->sections;

    ctx->sections = section;
    return ctx->section_count++;
}

void elf_write_section_data(ELFContext *ctx, int section_id, const void *data, size_t len)
{
    if (!ctx || !data || len == 0)
    {
        return;
    }

    // find section by id (sections are stored in reverse order)
    int current_id = ctx->section_count - 1;
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next, current_id--)
    {
        if (current_id == section_id)
        {
            // ensure capacity
            if (sec->size + len > sec->capacity)
            {
                size_t new_capacity = sec->capacity == 0 ? 4096 : sec->capacity * 2;
                while (new_capacity < sec->size + len)
                {
                    new_capacity *= 2;
                }

                uint8_t *new_data = realloc(sec->data, new_capacity);
                if (!new_data)
                {
                    return;
                }

                sec->data = new_data;
                sec->capacity = new_capacity;
            }

            // append data
            memcpy(sec->data + sec->size, data, len);
            sec->size += len;
            return;
        }
    }
}

void elf_add_symbol(ELFContext *ctx, const char *name, uint64_t value, int section_id, bool is_global, bool is_function)
{
    if (!ctx || !name)
    {
        return;
    }

    ELFSymbol *symbol = malloc(sizeof(ELFSymbol));
    if (!symbol)
    {
        return;
    }

    symbol->name = strdup(name);
    symbol->value = value;
    symbol->section_id = section_id;
    symbol->is_global = is_global;
    symbol->is_function = is_function;
    symbol->next = ctx->symbols;

    ctx->symbols = symbol;
    ctx->symbol_count++;
}

void elf_add_relocation(ELFContext *ctx, int section_id, uint64_t offset, const char *symbol_name, ELFX86_64RelocType type)
{
    if (!ctx || !symbol_name)
    {
        return;
    }

    ELFRelocation *reloc = malloc(sizeof(ELFRelocation));
    if (!reloc)
    {
        return;
    }

    reloc->section_id = section_id;
    reloc->offset = offset;
    reloc->symbol_name = strdup(symbol_name);
    reloc->type = type;
    reloc->next = ctx->relocations;

    ctx->relocations = reloc;
    ctx->reloc_count++;
}

int elf_write_to_file(ELFContext *ctx, const char *output_path)
{
    if (!ctx || !output_path)
    {
        return -1;
    }

    FILE *f = fopen(output_path, "wb");
    if (!f)
    {
        return -1;
    }

    // for now, just write a minimal elf header
    // full implementation will build proper section headers, symbol table, etc.
    
    Elf64_Ehdr ehdr = {0};
    
    // elf magic
    ehdr.e_ident[0] = 0x7F;
    ehdr.e_ident[1] = 'E';
    ehdr.e_ident[2] = 'L';
    ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2; // 64-bit
    ehdr.e_ident[5] = 1; // little endian
    ehdr.e_ident[6] = 1; // elf version
    ehdr.e_ident[7] = 0; // system v abi
    
    ehdr.e_type = 1;      // relocatable
    ehdr.e_machine = 0x3E; // x86-64
    ehdr.e_version = 1;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    
    fwrite(&ehdr, sizeof(ehdr), 1, f);
    
    fclose(f);
    return 0;
}
