#include "backend/codegen.h"

void backend_codegen_result_init(BackendCodegenResult *result)
{
    result->text.kind   = BACKEND_SECTION_TEXT;
    result->rodata.kind = BACKEND_SECTION_RODATA;
    result->data.kind   = BACKEND_SECTION_DATA;

    codebuf_init(&result->text.buffer);
    codebuf_init(&result->rodata.buffer);
    codebuf_init(&result->data.buffer);

    backend_label_list_init(&result->labels);
    backend_reloc_list_init(&result->relocs);
}

void backend_codegen_result_destroy(BackendCodegenResult *result)
{
    if (!result)
    {
        return;
    }

    codebuf_free(&result->text.buffer);
    codebuf_free(&result->rodata.buffer);
    codebuf_free(&result->data.buffer);

    backend_label_list_destroy(&result->labels);
    backend_reloc_list_destroy(&result->relocs);
}
