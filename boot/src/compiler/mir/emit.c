#include "compiler/mir/emit.h"
#include "compiler/mir/of/elf.h"
#include <stdlib.h>

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

        // add symbol
        elf_add_symbol(ctx->elf, global->name, 0, section, global->is_exported, false);

        // write initial data (simplified - would need proper serialization)
        if (global->kind != MIR_GLOBAL_UNINIT)
        {
            // placeholder: write zeros for now
            uint8_t zero = 0;
            elf_write_section_data(ctx->elf, section, &zero, 1);
        }
    }

    return 0;
}

static int emit_function_stub(EmitContext *ctx, MIRFunction *func)
{
    // for now, just add function symbol
    // actual code generation will be implemented later
    elf_add_symbol(ctx->elf, func->name, 0, ctx->text_section, func->is_exported, true);

    // placeholder: emit a ret instruction (0xC3 in x86-64)
    uint8_t ret_instruction = 0xC3;
    elf_write_section_data(ctx->elf, ctx->text_section, &ret_instruction, 1);

    return 0;
}

static int emit_functions(EmitContext *ctx, MIRModule *module)
{
    // emit all functions
    for (MIRFunction *func = module->functions; func; func = func->next)
    {
        if (emit_function_stub(ctx, func) < 0)
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
    // for now, executable generation is same as object file
    // later, this will link and produce a proper executable
    return mir_emit_object(module, target, output_path);
}
