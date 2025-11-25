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

// build string table from section names
static size_t build_shstrtab(ELFContext *ctx, uint8_t **out_data)
{
    // calculate total size needed
    size_t total_size = 1; // null byte at start
    
    // add null section name
    total_size += 1;
    
    // add section names
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        total_size += strlen(sec->name) + 1;
    }
    
    // add .shstrtab name itself
    total_size += strlen(".shstrtab") + 1;
    
    // allocate
    uint8_t *data = malloc(total_size);
    if (!data)
    {
        return 0;
    }
    
    // build string table
    size_t offset = 0;
    data[offset++] = 0; // null byte
    
    // write section names and record their offsets
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        sec->name[0] = offset; // store offset in first byte temporarily (hack)
        strcpy((char *)(data + offset), sec->name);
        offset += strlen(sec->name) + 1;
    }
    
    *out_data = data;
    return total_size;
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

    // count sections (including null, shstrtab)
    int section_count = 2; // null + shstrtab
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        section_count++;
    }
    
    // build section string table
    uint8_t *shstrtab_data = NULL;
    size_t shstrtab_size = build_shstrtab(ctx, &shstrtab_data);
    
    // write elf header
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
    ehdr.e_shnum = section_count;
    ehdr.e_shstrndx = section_count - 1; // shstrtab is last section
    ehdr.e_shoff = sizeof(Elf64_Ehdr); // sections follow header
    
    fwrite(&ehdr, sizeof(ehdr), 1, f);
    
    // calculate section data offsets
    uint64_t data_offset = sizeof(Elf64_Ehdr) + (section_count * sizeof(Elf64_Shdr));
    
    // write section headers
    // null section
    Elf64_Shdr null_shdr = {0};
    fwrite(&null_shdr, sizeof(null_shdr), 1, f);
    
    // regular sections
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        Elf64_Shdr shdr = {0};
        shdr.sh_name = (uint32_t)(uintptr_t)sec->name[0]; // offset we stored earlier
        shdr.sh_type = sec->type;
        shdr.sh_flags = sec->flags;
        shdr.sh_addr = 0;
        shdr.sh_offset = data_offset;
        shdr.sh_size = sec->size;
        shdr.sh_link = 0;
        shdr.sh_info = 0;
        shdr.sh_addralign = 1;
        shdr.sh_entsize = 0;
        
        fwrite(&shdr, sizeof(shdr), 1, f);
        data_offset += sec->size;
    }
    
    // shstrtab section
    Elf64_Shdr shstrtab_shdr = {0};
    shstrtab_shdr.sh_name = 1; // ".shstrtab" name offset
    shstrtab_shdr.sh_type = ELF_SHT_STRTAB;
    shstrtab_shdr.sh_flags = 0;
    shstrtab_shdr.sh_offset = data_offset;
    shstrtab_shdr.sh_size = shstrtab_size;
    shstrtab_shdr.sh_addralign = 1;
    fwrite(&shstrtab_shdr, sizeof(shstrtab_shdr), 1, f);
    
    // write section data
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        if (sec->data && sec->size > 0)
        {
            fwrite(sec->data, 1, sec->size, f);
        }
    }
    
    // write shstrtab data
    fwrite(shstrtab_data, 1, shstrtab_size, f);
    
    free(shstrtab_data);
    fclose(f);
    return 0;
}
