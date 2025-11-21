#include "backend/object/elf64.h"
#include "backend/codegen.h"
#include "backend/reloc.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t align_up(size_t value, size_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }
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
    {
        return NULL;
    }
    for (size_t i = 0; i < result->labels.count; i++)
    {
        if (strcmp(result->labels.items[i].name, name) == 0)
        {
            return &result->labels.items[i];
        }
    }
    return NULL;
}

static size_t section_base_vaddr(BackendSectionKind kind, size_t text_vaddr, size_t rodata_vaddr, size_t data_vaddr)
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

static bool apply_relocations(BackendCodegenResult *result, size_t text_vaddr, size_t rodata_vaddr, size_t data_vaddr)
{
    for (size_t i = 0; i < result->relocs.count; i++)
    {
        BackendReloc *reloc = &result->relocs.items[i];
        CodeBuffer   *buf   = get_buffer(result, reloc->section);
        if (!buf)
        {
            return false;
        }

        const BackendLabel *label = find_label(result, reloc->symbol);
        if (!label)
        {
            return false;
        }

        size_t symbol_addr = section_base_vaddr(label->section, text_vaddr, rodata_vaddr, data_vaddr) + label->offset;
        size_t place_addr  = section_base_vaddr(reloc->section, text_vaddr, rodata_vaddr, data_vaddr) + reloc->offset;

        switch (reloc->type)
        {
        case BACKEND_RELOC_X86_64_PC32:
        {
            if (reloc->offset + 4 > buf->size)
            {
                return false;
            }
            int64_t value                = (int64_t)symbol_addr + reloc->addend - (int64_t)place_addr;
            buf->data[reloc->offset + 0] = (uint8_t)(value & 0xFF);
            buf->data[reloc->offset + 1] = (uint8_t)((value >> 8) & 0xFF);
            buf->data[reloc->offset + 2] = (uint8_t)((value >> 16) & 0xFF);
            buf->data[reloc->offset + 3] = (uint8_t)((value >> 24) & 0xFF);
            break;
        }
        case BACKEND_RELOC_ABSOLUTE64:
        {
            if (reloc->offset + 8 > buf->size)
            {
                return false;
            }
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

typedef struct SectionBuilder
{
    Elf64_Shdr shdr;
} SectionBuilder;

static uint32_t shstrtab_add(CodeBuffer *buf, const char *name)
{
    if (!name)
    {
        name = "";
    }
    uint32_t offset = (uint32_t)buf->size;
    for (const char *p = name; *p; ++p)
    {
        codebuf_emit_byte(buf, (uint8_t)*p);
    }
    codebuf_emit_byte(buf, 0);
    return offset;
}

static bool elf64_write_executable(const Target *target, BackendCodegenResult *result, const char *path)
{
    const size_t page_size  = 0x1000;
    const size_t image_base = 0x400000;

    size_t text_size   = result->text.buffer.size;
    size_t rodata_size = result->rodata.buffer.size;
    size_t data_size   = result->data.buffer.size;

    size_t code_offset = page_size;
    size_t text_end    = code_offset + text_size;

    size_t rodata_offset = 0;
    if (rodata_size > 0)
    {
        rodata_offset = align_up(text_end, page_size);
    }

    size_t rodata_end = (rodata_size > 0) ? (rodata_offset + rodata_size) : text_end;

    size_t data_offset = 0;
    if (data_size > 0)
    {
        data_offset = align_up(rodata_end, page_size);
    }

    size_t text_vaddr   = image_base + code_offset;
    size_t rodata_vaddr = (rodata_size > 0) ? image_base + rodata_offset : 0;
    size_t data_vaddr   = (data_size > 0) ? image_base + data_offset : 0;

    // Entry point
    const char         *entry_label_name = target->runtime ? target->runtime->entry_label : NULL;
    const BackendLabel *entry_label      = find_label(result, entry_label_name ? entry_label_name : "_start");
    if (!entry_label || entry_label->section != BACKEND_SECTION_TEXT)
    {
        return false;
    }

    size_t entry_addr = text_vaddr + entry_label->offset;

    if (!apply_relocations(result, text_vaddr, rodata_vaddr, data_vaddr))
    {
        return false;
    }

    FILE *f = fopen(path, "wb");
    if (!f)
    {
        return false;
    }

    uint16_t phnum = 1;
    if (rodata_size > 0)
    {
        phnum++;
    }
    if (data_size > 0)
    {
        phnum++;
    }

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

    if (data_size > 0)
    {
        Elf64_Phdr phdr_data;
        memset(&phdr_data, 0, sizeof(phdr_data));
        phdr_data.p_type   = PT_LOAD;
        phdr_data.p_flags  = PF_R | PF_W;
        phdr_data.p_offset = data_offset;
        phdr_data.p_vaddr  = data_vaddr;
        phdr_data.p_paddr  = data_vaddr;
        phdr_data.p_filesz = data_size;
        phdr_data.p_memsz  = data_size;
        phdr_data.p_align  = page_size;
        fwrite(&phdr_data, sizeof(phdr_data), 1, f);
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
    if (rodata_size > 0)
    {
        while (current_offset < rodata_offset)
        {
            fputc(0, f);
            current_offset++;
        }
        fwrite(result->rodata.buffer.data, 1, rodata_size, f);
        current_offset = rodata_offset + rodata_size;
    }

    if (data_size > 0)
    {
        while (current_offset < data_offset)
        {
            fputc(0, f);
            current_offset++;
        }
        fwrite(result->data.buffer.data, 1, data_size, f);
        current_offset = data_offset + data_size;
    }

    // Build section headers and string table
    CodeBuffer shstrtab;
    codebuf_init(&shstrtab);
    codebuf_emit_byte(&shstrtab, 0); // null entry

    size_t section_count = 1; // null
    if (text_size > 0)
    {
        section_count++;
    }
    if (rodata_size > 0)
    {
        section_count++;
    }
    if (data_size > 0)
    {
        section_count++;
    }
    size_t shstr_index = section_count;
    section_count++;

    SectionBuilder *sections = calloc(section_count, sizeof(SectionBuilder));
    if (!sections)
    {
        codebuf_free(&shstrtab);
        fclose(f);
        return false;
    }

    size_t sec_idx = 1;
    if (text_size > 0)
    {
        SectionBuilder *sec    = &sections[sec_idx++];
        sec->shdr.sh_name      = shstrtab_add(&shstrtab, ".text");
        sec->shdr.sh_type      = SHT_PROGBITS;
        sec->shdr.sh_flags     = SHF_ALLOC | SHF_EXECINSTR;
        sec->shdr.sh_addr      = text_vaddr;
        sec->shdr.sh_offset    = code_offset;
        sec->shdr.sh_size      = text_size;
        sec->shdr.sh_addralign = 16;
    }
    if (rodata_size > 0)
    {
        SectionBuilder *sec    = &sections[sec_idx++];
        sec->shdr.sh_name      = shstrtab_add(&shstrtab, ".rodata");
        sec->shdr.sh_type      = SHT_PROGBITS;
        sec->shdr.sh_flags     = SHF_ALLOC;
        sec->shdr.sh_addr      = rodata_vaddr;
        sec->shdr.sh_offset    = rodata_offset;
        sec->shdr.sh_size      = rodata_size;
        sec->shdr.sh_addralign = 16;
    }
    if (data_size > 0)
    {
        SectionBuilder *sec    = &sections[sec_idx++];
        sec->shdr.sh_name      = shstrtab_add(&shstrtab, ".data");
        sec->shdr.sh_type      = SHT_PROGBITS;
        sec->shdr.sh_flags     = SHF_ALLOC | SHF_WRITE;
        sec->shdr.sh_addr      = data_vaddr;
        sec->shdr.sh_offset    = data_offset;
        sec->shdr.sh_size      = data_size;
        sec->shdr.sh_addralign = 16;
    }

    SectionBuilder *shstr_sec    = &sections[shstr_index];
    shstr_sec->shdr.sh_name      = shstrtab_add(&shstrtab, ".shstrtab");
    shstr_sec->shdr.sh_type      = SHT_STRTAB;
    shstr_sec->shdr.sh_flags     = 0;
    shstr_sec->shdr.sh_addralign = 1;

    size_t shstrtab_offset = current_offset;
    fwrite(shstrtab.data, 1, shstrtab.size, f);
    current_offset += shstrtab.size;

    size_t shoff = align_up(current_offset, 8);
    while (current_offset < shoff)
    {
        fputc(0, f);
        current_offset++;
    }

    shstr_sec->shdr.sh_offset = shstrtab_offset;
    shstr_sec->shdr.sh_size   = shstrtab.size;

    ehdr.e_shoff     = shoff;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum     = (uint16_t)section_count;
    ehdr.e_shstrndx  = (uint16_t)shstr_index;

    // rewrite ELF header with updated section info
    fseek(f, 0, SEEK_SET);
    fwrite(&ehdr, sizeof(ehdr), 1, f);

    fseek(f, shoff, SEEK_SET);
    for (size_t i = 0; i < section_count; i++)
    {
        fwrite(&sections[i].shdr, sizeof(Elf64_Shdr), 1, f);
    }

    fclose(f);
    codebuf_free(&shstrtab);
    free(sections);
    return true;
}

static ObjectWriter g_elf_writer = {
    .write_executable = elf64_write_executable,
};

const ObjectWriter *backend_object_writer_elf64(void)
{
    return &g_elf_writer;
}
