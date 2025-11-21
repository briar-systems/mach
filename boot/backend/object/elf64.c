#include "elf64.h"
#include "../codegen.h"
#include "../reloc.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t align_up(size_t value, size_t alignment)
{
    if (alignment == 0)
        return value;
    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

static CodeBuffer *get_buffer(BackendCodegenResult *result, BackendSectionKind kind)
{
    switch (kind)
    {
    case BACKEND_SECTION_TEXT:
        return &result->text.buffer;
    case BACKEND_SECTION_RODATA:
        return &result->rodata.buffer;
    case BACKEND_SECTION_DATA:
        return &result->data.buffer;
    default:
        return NULL;
    }
}

static const BackendLabel *find_label(const BackendCodegenResult *result, const char *name)
{
    if (!name)
        return NULL;
    for (size_t i = 0; i < result->labels.count; i++)
    {
        if (strcmp(result->labels.items[i].name, name) == 0)
            return &result->labels.items[i];
    }
    return NULL;
}

static size_t section_base_vaddr(BackendSectionKind kind,
                                 size_t text_vaddr,
                                 size_t rodata_vaddr,
                                 size_t data_vaddr)
{
    switch (kind)
    {
    case BACKEND_SECTION_TEXT:
        return text_vaddr;
    case BACKEND_SECTION_RODATA:
        return rodata_vaddr;
    case BACKEND_SECTION_DATA:
        return data_vaddr;
    default:
        return 0;
    }
}

static bool apply_relocations(BackendCodegenResult *result,
                              size_t text_vaddr,
                              size_t rodata_vaddr,
                              size_t data_vaddr)
{
    for (size_t i = 0; i < result->relocs.count; i++)
    {
        BackendReloc *reloc = &result->relocs.items[i];
        CodeBuffer   *buf   = get_buffer(result, reloc->section);
        if (!buf)
            return false;

        const BackendLabel *label = find_label(result, reloc->symbol);
        if (!label)
            return false;

        size_t symbol_addr = section_base_vaddr(label->section, text_vaddr, rodata_vaddr, data_vaddr) + label->offset;
        size_t place_addr  = section_base_vaddr(reloc->section, text_vaddr, rodata_vaddr, data_vaddr) + reloc->offset;

        switch (reloc->type)
        {
        case BACKEND_RELOC_X86_64_PC32:
        {
            if (reloc->offset + 4 > buf->size)
                return false;
            int64_t value = (int64_t)symbol_addr + reloc->addend - (int64_t)place_addr;
            buf->data[reloc->offset + 0] = (uint8_t)(value & 0xFF);
            buf->data[reloc->offset + 1] = (uint8_t)((value >> 8) & 0xFF);
            buf->data[reloc->offset + 2] = (uint8_t)((value >> 16) & 0xFF);
            buf->data[reloc->offset + 3] = (uint8_t)((value >> 24) & 0xFF);
            break;
        }
        case BACKEND_RELOC_ABSOLUTE64:
        {
            if (reloc->offset + 8 > buf->size)
                return false;
            int64_t value = (int64_t)symbol_addr + reloc->addend;
            for (int byte = 0; byte < 8; byte++)
            {
                buf->data[reloc->offset + byte] = (uint8_t)((value >> (byte * 8)) & 0xFF);
            }
            break;
        }
        default:
            return false;
        }
    }

    return true;
}

static bool elf64_write_executable(const BackendTarget *target, BackendCodegenResult *result, const char *path)
{
    const size_t page_size  = 0x1000;
    const size_t image_base = 0x400000;

    size_t text_size   = result->text.buffer.size;
    size_t rodata_size = result->rodata.buffer.size;

    // Layout
    size_t code_offset   = page_size;
    size_t rodata_offset = align_up(code_offset + text_size, page_size);
    size_t text_vaddr    = image_base + code_offset;
    size_t rodata_vaddr  = image_base + rodata_offset;
    size_t data_vaddr    = 0; // not used yet

    // Entry point
    const char *entry_label_name = target->runtime ? target->runtime->entry_label : NULL;
    const BackendLabel *entry_label = find_label(result, entry_label_name ? entry_label_name : "_start");
    if (!entry_label || entry_label->section != BACKEND_SECTION_TEXT)
        return false;

    size_t entry_addr = text_vaddr + entry_label->offset;

    if (!apply_relocations(result, text_vaddr, rodata_vaddr, data_vaddr))
        return false;

    FILE *f = fopen(path, "wb");
    if (!f)
        return false;

    uint16_t phnum = rodata_size > 0 ? 2 : 1;

    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    ehdr.e_ident[EI_MAG0]    = ELFMAG0;
    ehdr.e_ident[EI_MAG1]    = ELFMAG1;
    ehdr.e_ident[EI_MAG2]    = ELFMAG2;
    ehdr.e_ident[EI_MAG3]    = ELFMAG3;
    ehdr.e_ident[EI_CLASS]   = ELFCLASS64;
    ehdr.e_ident[EI_DATA]    = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI]   = ELFOSABI_SYSV;
    ehdr.e_type              = ET_EXEC;
    ehdr.e_machine           = EM_X86_64;
    ehdr.e_version           = EV_CURRENT;
    ehdr.e_entry             = entry_addr;
    ehdr.e_phoff             = sizeof(Elf64_Ehdr);
    ehdr.e_ehsize            = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize         = sizeof(Elf64_Phdr);
    ehdr.e_phnum             = phnum;

    fwrite(&ehdr, sizeof(ehdr), 1, f);

    // Program headers
    Elf64_Phdr phdr_text;
    memset(&phdr_text, 0, sizeof(phdr_text));
    phdr_text.p_type   = PT_LOAD;
    phdr_text.p_flags  = PF_R | PF_X;
    phdr_text.p_offset = 0;
    phdr_text.p_vaddr  = image_base;
    phdr_text.p_paddr  = image_base;
    phdr_text.p_filesz = code_offset + text_size;
    phdr_text.p_memsz  = code_offset + text_size;
    phdr_text.p_align  = page_size;
    fwrite(&phdr_text, sizeof(phdr_text), 1, f);

    if (rodata_size > 0)
    {
        Elf64_Phdr phdr_rodata;
        memset(&phdr_rodata, 0, sizeof(phdr_rodata));
        phdr_rodata.p_type   = PT_LOAD;
        phdr_rodata.p_flags  = PF_R;
        phdr_rodata.p_offset = rodata_offset;
        phdr_rodata.p_vaddr  = rodata_vaddr;
        phdr_rodata.p_paddr  = rodata_vaddr;
        phdr_rodata.p_filesz = rodata_size;
        phdr_rodata.p_memsz  = rodata_size;
        phdr_rodata.p_align  = page_size;
        fwrite(&phdr_rodata, sizeof(phdr_rodata), 1, f);
    }

    // Pad up to code offset
    size_t current_offset = sizeof(Elf64_Ehdr) + phnum * sizeof(Elf64_Phdr);
    if (current_offset > code_offset)
    {
        fclose(f);
        return false;
    }
    while (current_offset < code_offset)
    {
        fputc(0, f);
        current_offset++;
    }

    // Write text section
    fwrite(result->text.buffer.data, 1, text_size, f);

    // Pad to rodata offset
    current_offset = code_offset + text_size;
    while (current_offset < rodata_offset)
    {
        fputc(0, f);
        current_offset++;
    }

    // Write rodata
    if (rodata_size > 0)
    {
        fwrite(result->rodata.buffer.data, 1, rodata_size, f);
    }

    fclose(f);
    return true;
}

static ObjectWriter g_elf_writer = {
    .write_executable = elf64_write_executable,
};

const ObjectWriter *backend_object_writer_elf64(void)
{
    return &g_elf_writer;
}
