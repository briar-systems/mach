#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdbool.h>
#include <stddef.h>

typedef struct PreprocessorConstant
{
    const char *name;
    long long   value;
} PreprocessorConstant;

typedef struct PreprocessorOutput
{
    bool  success;
    char *source;
    char *message;
    int   line;
} PreprocessorOutput;

void preprocessor_output_init(PreprocessorOutput *output);
void preprocessor_output_dnit(PreprocessorOutput *output);

bool preprocessor_run(const char *source, const PreprocessorConstant *constants, size_t constant_count, PreprocessorOutput *output);

#endif
