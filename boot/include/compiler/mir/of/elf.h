#ifndef MIR_OF_ELF_H
#define MIR_OF_ELF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// elf64 object file format

// section flags
#define ELF_SHF_WRITE     0x1
#define ELF_SHF_ALLOC     0x2
#define ELF_SHF_EXECINSTR 0x4

// section types
typedef enum ELFSectionType
{
    ELF_SHT_NULL,
    ELF_SHT_PROGBITS,
    ELF_SHT_SYMTAB,
    ELF_SHT_STRTAB,
    ELF_SHT_RELA,
    ELF_SHT_NOBITS,
} ELFSectionType;

// relocation types (x86_64)
typedef enum ELFX86_64RelocType
{
    ELF_R_X86_64_NONE,
    ELF_R_X86_64_64,
    ELF_R_X86_64_PC32,
    ELF_R_X86_64_PLT32,
} ELFX86_64RelocType;

// elf context (opaque, defined in implementation)
typedef struct ELFContext ELFContext;

// lifecycle
ELFContext *elf_context_create(const char *filename);
void        elf_context_destroy(ELFContext *ctx);

// section management
int  elf_create_section(ELFContext *ctx, const char *name, ELFSectionType type, uint32_t flags);
void elf_write_section_data(ELFContext *ctx, int section_id, const void *data, size_t len);

// symbol table
void elf_add_symbol(ELFContext *ctx, const char *name, uint64_t value, int section_id, bool is_global, bool is_function);

// relocations
void elf_add_relocation(ELFContext *ctx, int section_id, uint64_t offset, const char *symbol_name, ELFX86_64RelocType type, int64_t addend);

// section queries
uint64_t elf_get_section_size(ELFContext *ctx, int section_id);

// finalize and write
int elf_write_to_file(ELFContext *ctx, const char *output_path);

#endif // MIR_OF_ELF_H
