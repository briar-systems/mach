#include "compiler/masm/of/elf.h"
#include "compiler/masm/isa/x86_64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ELF Definitions
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
#define PF_X    1
#define PF_W    2
#define PF_R    4

#define ET_EXEC 2
#define EM_X86_64 62

// Simple ELF writer for executable
int masm_elf_write(Masm *masm, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;

    MasmSection *text = masm_get_section(masm, ".text");
    if (!text)
    {
        fclose(f);
        return -1;
    }

    // Pass 1: Calculate offsets and record label positions
    size_t current_offset = 0;
    // We need to store instruction offsets to calculate relative jumps later
    // For now, assume we can re-calculate or store them.
    // Let's store label offsets in the symbol table.
    
    // We need to iterate instructions and update symbol offsets if we see a label definition.
    // But wait, MASM_OP_LABEL *is* the definition.
    
    // We also need to know the size of each instruction.
    // masm_x86_encode returns size. We can use a dummy buffer.
    uint8_t dummy_buffer[16];
    
    // To avoid re-encoding everything twice, we could store instruction sizes or offsets.
    // For simplicity, we'll re-encode.
    
    // Map label names to offsets.
    // Since we don't have a hash map handy in C easily without pulling in deps,
    // and we have a small number of labels (main, _start), we can just search.
    // Actually, `masm` has a symbol table `masm->symbols`.
    // We should update `MasmSymbol` with the offset.
    
    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction *inst = &text->instructions[i];
        if (inst->opcode == MASM_OP_LABEL)
        {
            // Update symbol offset
            if (inst->operands[0].label)
            {
                MasmSymbol *sym = masm_get_symbol(masm, inst->operands[0].label);
                if (sym)
                {
                    sym->offset = current_offset;
                }
                // Also handle local labels if any (not yet)
            }
        }
        else
        {
            size_t sz = masm_x86_encode(*inst, dummy_buffer, sizeof(dummy_buffer));
            current_offset += sz;
        }
    }
    
    size_t total_code_size = current_offset;

    // Buffer for code
    uint8_t *code_buffer = malloc(total_code_size);
    size_t code_size = 0;
    
    // Find entry point (_start)
    uint64_t entry_offset = 0;
    MasmSymbol *start_sym = masm_get_symbol(masm, "_start");
    if (start_sym)
    {
        entry_offset = start_sym->offset;
    }

    // Pass 2: Encode with label resolution
    for (size_t i = 0; i < text->inst_count; i++)
    {
        MasmInstruction inst = text->instructions[i];
        if (inst.opcode == MASM_OP_LABEL)
        {
            continue;
        }
        
        // Check for CALL/JMP with label operand
        if (inst.opcode == MASM_OP_CALL && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            // Calculate relative offset
            MasmSymbol *target = masm_get_symbol(masm, inst.operands[0].label);
            if (target)
            {
                // Target offset - (Current offset + Instruction size)
                // Instruction size for CALL rel32 is 5 bytes
                int32_t rel = (int32_t)(target->offset - (code_size + 5));
                
                // We need to manually encode CALL here or pass the 'rel' to encoder?
                // masm_x86_encode doesn't take 'rel'.
                // We can hack it: change operand to IMM with the relative value?
                // No, CALL expects LABEL or IMM?
                // Let's manually encode CALL here to override x86_64.c behavior
                code_buffer[code_size++] = 0xE8;
                
                // emit int32
                for (int k = 0; k < 4; k++)
                {
                    code_buffer[code_size++] = (rel >> (k * 8)) & 0xFF;
                }
                continue;
            }
        }
        
        code_size += masm_x86_encode(inst, code_buffer + code_size, total_code_size - code_size);
    }

    // Base address for executable
    uint64_t base_addr = 0x400000;
    // Align to page size (4096)
    uint64_t code_offset = 0x1000; // simplified alignment
    uint64_t entry_addr = base_addr + code_offset + entry_offset;

    // 2. Write ELF Header
    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    ehdr.e_ident[0] = 0x7F;
    ehdr.e_ident[1] = 'E';
    ehdr.e_ident[2] = 'L';
    ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2; // Class: 64-bit
    ehdr.e_ident[5] = 1; // Data: 2's complement, little endian
    ehdr.e_ident[6] = 1; // Version
    ehdr.e_ident[7] = 0; // OS ABI: System V
    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_X86_64;
    ehdr.e_version = 1;
    ehdr.e_entry = entry_addr;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_shoff = 0; // No section headers for now
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 1;
    ehdr.e_shentsize = 0;
    ehdr.e_shnum = 0;
    ehdr.e_shstrndx = 0;

    fwrite(&ehdr, 1, sizeof(ehdr), f);

    // 3. Write Program Header
    Elf64_Phdr phdr;
    memset(&phdr, 0, sizeof(phdr));
    phdr.p_type = PT_LOAD;
    phdr.p_flags = PF_R | PF_X; // Read + Execute
    phdr.p_offset = code_offset; // Offset in file
    phdr.p_vaddr = base_addr + code_offset;
    phdr.p_paddr = base_addr + code_offset;
    phdr.p_filesz = code_size;
    phdr.p_memsz = code_size;
    phdr.p_align = 0x1000;

    fwrite(&phdr, 1, sizeof(phdr), f);

    // Pad to code offset
    fseek(f, code_offset, SEEK_SET);

    // 4. Write Code
    fwrite(code_buffer, 1, code_size, f);

    free(code_buffer);
    fclose(f);
    return 0;
}
