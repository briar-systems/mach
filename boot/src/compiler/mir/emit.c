#include "compiler/mir/emit.h"
#include "compiler/mir/of/elf.h"
#include "compiler/mir/codegen/x86_64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// emission context
typedef struct
{
    const MIRTarget *target;
    ELFContext      *elf;
    int              text_section;
    int              data_section;
    int              rodata_section;
    int              bss_section;
} EmitContext;

static EmitContext *emit_context_create(const MIRTarget *target)
{
    EmitContext *ctx = malloc(sizeof(EmitContext));
    if (!ctx)
    {
        return NULL;
    }

    ctx->target = target;
    ctx->elf = NULL;
    ctx->text_section = -1;
    ctx->data_section = -1;
    ctx->rodata_section = -1;
    ctx->bss_section = -1;

    return ctx;
}

static void emit_context_destroy(EmitContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->elf)
    {
        elf_context_destroy(ctx->elf);
    }

    free(ctx);
}

static int emit_create_sections(EmitContext *ctx)
{
    // create standard elf sections
    ctx->text_section = elf_create_section(ctx->elf, ".text", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_EXECINSTR);
    ctx->data_section = elf_create_section(ctx->elf, ".data", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    ctx->rodata_section = elf_create_section(ctx->elf, ".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    ctx->bss_section = elf_create_section(ctx->elf, ".bss", ELF_SHT_NOBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);

    if (ctx->text_section < 0 || ctx->data_section < 0 || ctx->rodata_section < 0 || ctx->bss_section < 0)
    {
        return -1;
    }

    return 0;
}

static int emit_globals(EmitContext *ctx, MIRModule *module)
{
    // emit global data declarations
    for (MIRGlobal *global = module->globals; global; global = global->next)
    {
        int section = ctx->bss_section;

        // determine section based on kind
        if (global->kind == MIR_GLOBAL_VAL)
        {
            section = ctx->rodata_section;
        }
        else if (global->kind == MIR_GLOBAL_VAR)
        {
            section = ctx->data_section;
        }

        // get current section offset for symbol
        uint64_t offset = elf_get_section_size(ctx->elf, section);

        // add symbol at current offset
        elf_add_symbol(ctx->elf, global->name, offset, section, global->is_exported, false);

        // write initial data
        if (global->kind != MIR_GLOBAL_UNINIT)
        {
            if (global->init_kind == MIR_INIT_STRING)
            {
                // string literal - write with null terminator
                size_t len = strlen(global->init.string_value);
                elf_write_section_data(ctx->elf, section, (const uint8_t *)global->init.string_value, len + 1);
            }
            else if (global->init_kind == MIR_INIT_FLOAT)
            {
                // float - write as double (8 bytes)
                double value = global->init.float_value;
                elf_write_section_data(ctx->elf, section, (const uint8_t *)&value, 8);
            }
            else if (global->init_kind == MIR_INIT_INT)
            {
                // integer/pointer - write 8 bytes
                uint64_t value = (uint64_t)global->init.int_value;
                elf_write_section_data(ctx->elf, section, (const uint8_t *)&value, 8);
            }
            else if (global->type->kind == MIR_TYPE_F32 || global->type->kind == MIR_TYPE_F64)
            {
                // fallback based on type if init_kind not set (should not happen with new code)
                double value = global->init.float_value;
                elf_write_section_data(ctx->elf, section, (const uint8_t *)&value, 8);
            }
            else if (global->type->kind == MIR_TYPE_I64 || global->type->kind == MIR_TYPE_U64 ||
                     global->type->kind == MIR_TYPE_I32 || global->type->kind == MIR_TYPE_U32 ||
                     global->type->kind == MIR_TYPE_I16 || global->type->kind == MIR_TYPE_U16 ||
                     global->type->kind == MIR_TYPE_I8  || global->type->kind == MIR_TYPE_U8  ||
                     global->type->kind == MIR_TYPE_PTR)
            {
                // integer/pointer - write 8 bytes
                uint64_t value = (uint64_t)global->init.int_value;
                elf_write_section_data(ctx->elf, section, (const uint8_t *)&value, 8);
            }
            else if (global->init.string_value)
            {
                // fallback for string
                size_t len = strlen(global->init.string_value);
                elf_write_section_data(ctx->elf, section, (const uint8_t *)global->init.string_value, len + 1);
            }
        }
    }

    return 0;
}

static int emit_function_code(EmitContext *ctx, MIRFunction *func)
{
    // create x86_64 codegen context
    X86_64_CodegenContext *codegen = x86_64_codegen_create();
    if (!codegen)
    {
        return -1;
    }

    // generate machine code
    if (x86_64_emit_function(codegen, func) < 0)
    {
        x86_64_codegen_destroy(codegen);
        return -1;
    }

    // get generated code
    size_t code_size = 0;
    uint8_t *code = x86_64_codegen_get_code(codegen, &code_size);

    // get current .text offset for symbol
    uint64_t text_offset = elf_get_section_size(ctx->elf, ctx->text_section);

    // add function symbol at current .text offset
    elf_add_symbol(ctx->elf, func->name, text_offset, ctx->text_section, func->is_exported, true);

    // write code to .text section
    if (code && code_size > 0)
    {
        elf_write_section_data(ctx->elf, ctx->text_section, code, code_size);
    }

    // transfer relocations from codegen to ELF (adjust offsets to section-relative)
    X86_64_Relocation *reloc = x86_64_codegen_get_relocations(codegen);
    while (reloc)
    {
        elf_add_relocation(ctx->elf, ctx->text_section, text_offset + reloc->offset, reloc->symbol_name, reloc->type, reloc->addend);
        reloc = reloc->next;
    }

    x86_64_codegen_destroy(codegen);
    return 0;
}

static int emit_functions(EmitContext *ctx, MIRModule *module)
{
    // emit all functions
    for (MIRFunction *func = module->functions; func; func = func->next)
    {
        if (emit_function_code(ctx, func) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int mir_emit_object(MIRModule *module, const MIRTarget *target, const char *output_path)
{
    if (!module || !target || !output_path)
    {
        return -1;
    }

    EmitContext *ctx = emit_context_create(target);
    if (!ctx)
    {
        return -1;
    }

    // create elf context
    ctx->elf = elf_context_create(output_path);
    if (!ctx->elf)
    {
        emit_context_destroy(ctx);
        return -1;
    }

    // create sections
    if (emit_create_sections(ctx) < 0)
    {
        emit_context_destroy(ctx);
        return -1;
    }

    // emit globals
    if (emit_globals(ctx, module) < 0)
    {
        emit_context_destroy(ctx);
        return -1;
    }

    // emit functions
    if (emit_functions(ctx, module) < 0)
    {
        emit_context_destroy(ctx);
        return -1;
    }

    // write to file
    int result = elf_write_to_file(ctx->elf, output_path);

    emit_context_destroy(ctx);
    return result;
}

int mir_emit_executable(MIRModule *module, const MIRTarget *target, const char *output_path)
{
    // emit object file first
    char temp_obj[256];
    snprintf(temp_obj, sizeof(temp_obj), "%s.o", output_path);
    
    if (mir_emit_object(module, target, temp_obj) < 0)
    {
        return -1;
    }

    // link using ld (simple approach for now)
    char link_cmd[512];
    snprintf(link_cmd, sizeof(link_cmd), "ld -o %s %s 2>/dev/null", output_path, temp_obj);
    
    int result = system(link_cmd);
    
    // clean up temp object file
    remove(temp_obj);
    
    return result == 0 ? 0 : -1;
}
