#ifndef BACKEND_CODEGEN_H
#define BACKEND_CODEGEN_H

#include "codebuf.h"
#include "reloc.h"

typedef struct BackendSection
{
    BackendSectionKind kind;
    CodeBuffer         buffer;
} BackendSection;

typedef struct BackendCodegenResult
{
    BackendSection    text;
    BackendSection    rodata;
    BackendSection    data;
    BackendLabelList  labels;
    BackendRelocList  relocs;
} BackendCodegenResult;

void backend_codegen_result_init(BackendCodegenResult *result);
void backend_codegen_result_destroy(BackendCodegenResult *result);

#endif // BACKEND_CODEGEN_H
