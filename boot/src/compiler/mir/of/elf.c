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

uint64_t elf_get_section_size(ELFContext *ctx, int section_id)
{
    if (!ctx)
    {
        return 0;
    }

    // find section by id (sections are stored in reverse order)
    int current_id = ctx->section_count - 1;
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next, current_id--)
    {
        if (current_id == section_id)
        {
            return sec->size;
        }
    }

    return 0;
}

// string table builder helper
typedef struct
{
    uint8_t *data;
    size_t   size;
    size_t   capacity;
} StringTable;

static void strtab_init(StringTable *st)
{
    st->capacity = 256;
    st->data = malloc(st->capacity);
    st->size = 1; // start with null byte
    st->data[0] = 0;
}

static uint32_t strtab_add(StringTable *st, const char *str)
{
    if (!str)
    {
        return 0;
    }
    
    size_t len = strlen(str) + 1;
    
    // ensure capacity
    while (st->size + len > st->capacity)
    {
        st->capacity *= 2;
        st->data = realloc(st->data, st->capacity);
    }
    
    uint32_t offset = st->size;
    strcpy((char *)(st->data + offset), str);
    st->size += len;
    
    return offset;
}

static void strtab_free(StringTable *st)
{
    if (st->data)
    {
        free(st->data);
        st->data = NULL;
    }
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

    // build string tables
    StringTable shstrtab, strtab;
    strtab_init(&shstrtab);
    strtab_init(&strtab);
    
    // count sections and reverse the order (they're stored backwards in linked list)
    int regular_section_count = 0;
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        regular_section_count++;
    }
    
    // create array of sections in proper order
    ELFSection **sections_array = malloc(regular_section_count * sizeof(ELFSection *));
    int idx = regular_section_count - 1;
    for (ELFSection *sec = ctx->sections; sec; sec = sec->next)
    {
        sections_array[idx--] = sec;
    }
    
    // add section names to shstrtab
    uint32_t *section_name_offsets = malloc(regular_section_count * sizeof(uint32_t));
    for (int i = 0; i < regular_section_count; i++)
    {
        section_name_offsets[i] = strtab_add(&shstrtab, sections_array[i]->name);
    }
    
    uint32_t symtab_name_offset = strtab_add(&shstrtab, ".symtab");
    uint32_t strtab_name_offset = strtab_add(&shstrtab, ".strtab");
    uint32_t shstrtab_name_offset = strtab_add(&shstrtab, ".shstrtab");
    
    // build symbol table and add symbol names to strtab
    int symbol_count = 1; // null symbol
    for (ELFSymbol *sym = ctx->symbols; sym; sym = sym->next)
    {
        symbol_count++;
    }
    
    Elf64_Sym *symbols = calloc(symbol_count, sizeof(Elf64_Sym));
    
    // null symbol at index 0 is already zeroed
    
    // add real symbols - local symbols first, then global symbols (ELF requirement)
    idx = 1;
    int first_global_idx = symbol_count; // will be updated when we find first global
    
    // first pass: add local symbols
    for (ELFSymbol *sym = ctx->symbols; sym; sym = sym->next)
    {
        if (!sym->is_global)
        {
            symbols[idx].st_name = strtab_add(&strtab, sym->name);
            symbols[idx].st_info = (0 << 4) | (sym->is_function ? 2 : 1); // STB_LOCAL
            symbols[idx].st_other = 0;
            symbols[idx].st_shndx = sym->section_id + 1; // +1 for null section
            symbols[idx].st_value = sym->value;
            symbols[idx].st_size = 0;
            idx++;
        }
    }
    
    // record where globals start
    first_global_idx = idx;
    
    // second pass: add global symbols
    for (ELFSymbol *sym = ctx->symbols; sym; sym = sym->next)
    {
        if (sym->is_global)
        {
            symbols[idx].st_name = strtab_add(&strtab, sym->name);
            symbols[idx].st_info = (1 << 4) | (sym->is_function ? 2 : 1); // STB_GLOBAL
            symbols[idx].st_other = 0;
            symbols[idx].st_shndx = sym->section_id + 1; // +1 for null section
            symbols[idx].st_value = sym->value;
            symbols[idx].st_size = 0;
            idx++;
        }
    }
    
    // count relocations per section
    int *reloc_counts = calloc(regular_section_count, sizeof(int));
    for (ELFRelocation *reloc = ctx->relocations; reloc; reloc = reloc->next)
    {
        if (reloc->section_id >= 0 && reloc->section_id < regular_section_count)
        {
            reloc_counts[reloc->section_id]++;
        }
    }
    
    // calculate section count
    int section_count = 1; // null section
    section_count += regular_section_count;
    
    // add relocation sections for sections that have relocations
    int rela_section_count = 0;
    for (int i = 0; i < regular_section_count; i++)
    {
        if (reloc_counts[i] > 0)
        {
            rela_section_count++;
        }
    }
    section_count += rela_section_count;
    
    section_count += 3; // symtab, strtab, shstrtab
    
    // calculate file layout
    uint64_t current_offset = sizeof(Elf64_Ehdr) + (section_count * sizeof(Elf64_Shdr));
    
    // data sections come first
    uint64_t *section_offsets = malloc(regular_section_count * sizeof(uint64_t));
    for (int i = 0; i < regular_section_count; i++)
    {
        section_offsets[i] = current_offset;
        current_offset += sections_array[i]->size;
    }
    
    // then relocation sections
    uint64_t *rela_offsets = calloc(regular_section_count, sizeof(uint64_t));
    uint32_t *rela_name_offsets = calloc(regular_section_count, sizeof(uint32_t));
    for (int i = 0; i < regular_section_count; i++)
    {
        if (reloc_counts[i] > 0)
        {
            char rela_name[256];
            snprintf(rela_name, sizeof(rela_name), ".rela%s", sections_array[i]->name);
            rela_name_offsets[i] = strtab_add(&shstrtab, rela_name);
            rela_offsets[i] = current_offset;
            current_offset += reloc_counts[i] * sizeof(Elf64_Rela);
        }
    }
    
    // then symtab
    uint64_t symtab_offset = current_offset;
    size_t symtab_size = symbol_count * sizeof(Elf64_Sym);
    current_offset += symtab_size;
    
    // then strtab
    uint64_t strtab_offset = current_offset;
    current_offset += strtab.size;
    
    // finally shstrtab
    uint64_t shstrtab_offset = current_offset;
    
    // write ELF header
    Elf64_Ehdr ehdr = {0};
    ehdr.e_ident[0] = 0x7F;
    ehdr.e_ident[1] = 'E';
    ehdr.e_ident[2] = 'L';
    ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2; // 64-bit
    ehdr.e_ident[5] = 1; // little endian
    ehdr.e_ident[6] = 1; // current version
    ehdr.e_ident[7] = 0; // system v abi
    
    ehdr.e_type = 1; // relocatable
    ehdr.e_machine = 0x3E; // x86-64
    ehdr.e_version = 1;
    ehdr.e_entry = 0;
    ehdr.e_phoff = 0; // no program headers
    ehdr.e_shoff = sizeof(Elf64_Ehdr);
    ehdr.e_flags = 0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum = section_count;
    ehdr.e_shstrndx = section_count - 1; // shstrtab is last
    
    fwrite(&ehdr, sizeof(ehdr), 1, f);
    
    // write section headers
    
    // null section header
    Elf64_Shdr shdr = {0};
    fwrite(&shdr, sizeof(shdr), 1, f);
    
    // data section headers
    for (int i = 0; i < regular_section_count; i++)
    {
        ELFSection *sec = sections_array[i];
        memset(&shdr, 0, sizeof(shdr));
        shdr.sh_name = section_name_offsets[i];
        shdr.sh_type = sec->type;
        shdr.sh_flags = sec->flags;
        shdr.sh_addr = 0;
        shdr.sh_offset = section_offsets[i];
        shdr.sh_size = sec->size;
        shdr.sh_link = 0;
        shdr.sh_info = 0;
        shdr.sh_addralign = (sec->type == ELF_SHT_NOBITS) ? 1 : 1;
        shdr.sh_entsize = 0;
        fwrite(&shdr, sizeof(shdr), 1, f);
    }
    
    // relocation section headers
    for (int i = 0; i < regular_section_count; i++)
    {
        if (reloc_counts[i] > 0)
        {
            memset(&shdr, 0, sizeof(shdr));
            shdr.sh_name = rela_name_offsets[i];
            shdr.sh_type = ELF_SHT_RELA;
            shdr.sh_flags = 0;
            shdr.sh_addr = 0;
            shdr.sh_offset = rela_offsets[i];
            shdr.sh_size = reloc_counts[i] * sizeof(Elf64_Rela);
            shdr.sh_link = section_count - 3; // link to symtab
            shdr.sh_info = i + 1; // section to apply relocations to (+1 for null)
            shdr.sh_addralign = 8;
            shdr.sh_entsize = sizeof(Elf64_Rela);
            fwrite(&shdr, sizeof(shdr), 1, f);
        }
    }
    
    // symtab section header
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_name = symtab_name_offset;
    shdr.sh_type = ELF_SHT_SYMTAB;
    shdr.sh_flags = 0;
    shdr.sh_addr = 0;
    shdr.sh_offset = symtab_offset;
    shdr.sh_size = symtab_size;
    shdr.sh_link = section_count - 2; // link to strtab (second to last)
    shdr.sh_info = first_global_idx; // index of first global symbol
    shdr.sh_addralign = 8;
    shdr.sh_entsize = sizeof(Elf64_Sym);
    fwrite(&shdr, sizeof(shdr), 1, f);
    
    // strtab section header
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_name = strtab_name_offset;
    shdr.sh_type = ELF_SHT_STRTAB;
    shdr.sh_flags = 0;
    shdr.sh_addr = 0;
    shdr.sh_offset = strtab_offset;
    shdr.sh_size = strtab.size;
    shdr.sh_link = 0;
    shdr.sh_info = 0;
    shdr.sh_addralign = 1;
    shdr.sh_entsize = 0;
    fwrite(&shdr, sizeof(shdr), 1, f);
    
    // shstrtab section header
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_name = shstrtab_name_offset;
    shdr.sh_type = ELF_SHT_STRTAB;
    shdr.sh_flags = 0;
    shdr.sh_addr = 0;
    shdr.sh_offset = shstrtab_offset;
    shdr.sh_size = shstrtab.size;
    shdr.sh_link = 0;
    shdr.sh_info = 0;
    shdr.sh_addralign = 1;
    shdr.sh_entsize = 0;
    fwrite(&shdr, sizeof(shdr), 1, f);
    
    // write section data
    for (int i = 0; i < regular_section_count; i++)
    {
        ELFSection *sec = sections_array[i];
        if (sec->type != ELF_SHT_NOBITS && sec->data && sec->size > 0)
        {
            fwrite(sec->data, 1, sec->size, f);
        }
    }
    
    // write relocation data
    for (int i = 0; i < regular_section_count; i++)
    {
        if (reloc_counts[i] > 0)
        {
            // collect relocations for this section
            Elf64_Rela *relas = calloc(reloc_counts[i], sizeof(Elf64_Rela));
            int rela_idx = 0;
            
            for (ELFRelocation *reloc = ctx->relocations; reloc; reloc = reloc->next)
            {
                if (reloc->section_id == i)
                {
                    // find symbol index
                    int sym_idx = 0;
                    int cur_idx = 1; // skip null symbol
                    
                    // check local symbols
                    for (ELFSymbol *sym = ctx->symbols; sym; sym = sym->next)
                    {
                        if (!sym->is_global)
                        {
                            if (strcmp(sym->name, reloc->symbol_name) == 0)
                            {
                                sym_idx = cur_idx;
                                break;
                            }
                            cur_idx++;
                        }
                    }
                    
                    // check global symbols if not found
                    if (sym_idx == 0)
                    {
                        cur_idx = first_global_idx;
                        for (ELFSymbol *sym = ctx->symbols; sym; sym = sym->next)
                        {
                            if (sym->is_global)
                            {
                                if (strcmp(sym->name, reloc->symbol_name) == 0)
                                {
                                    sym_idx = cur_idx;
                                    break;
                                }
                                cur_idx++;
                            }
                        }
                    }
                    
                    relas[rela_idx].r_offset = reloc->offset;
                    relas[rela_idx].r_info = ((uint64_t)sym_idx << 32) | (uint64_t)reloc->type;
                    relas[rela_idx].r_addend = 0;
                    rela_idx++;
                }
            }
            
            fwrite(relas, sizeof(Elf64_Rela), reloc_counts[i], f);
            free(relas);
        }
    }
    
    // write symtab
    fwrite(symbols, sizeof(Elf64_Sym), symbol_count, f);
    
    // write strtab
    fwrite(strtab.data, 1, strtab.size, f);
    
    // write shstrtab
    fwrite(shstrtab.data, 1, shstrtab.size, f);
    
    // cleanup
    free(sections_array);
    free(section_name_offsets);
    free(section_offsets);
    free(symbols);
    free(reloc_counts);
    free(rela_offsets);
    free(rela_name_offsets);
    strtab_free(&shstrtab);
    strtab_free(&strtab);
    fclose(f);
    
    return 0;
}
