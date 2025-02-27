#include "mach.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

void val_list_init(ValList *list)
{
    list->len = 0;
    list->cap = 0;
    list->vals = NULL;
}

void val_list_free(ValList *list)
{
    free(list->vals);
    list->len = 0;
    list->cap = 0;
    list->vals = NULL;
}

void val_list_push(ValList *list, val_t val)
{
    if (list->len + 1 > list->cap)
    {
        list->cap = list->cap < 8 ? 8 : list->cap * 2;
        list->vals = realloc(list->vals, list->cap * sizeof(val_t));

        if (list->vals == NULL)
        {
            fprintf(stderr, "failed to allocate memory for val list\n");
            exit(EXIT_FAILURE);
        }
    }
    list->vals[list->len++] = val;
}

val_t val_list_pop(ValList *list)
{
    if (list->len == 0)
    {
        fprintf(stderr, "val list is empty\n");
        exit(EXIT_FAILURE);
    }
    return list->vals[--list->len];
}

void chunk_init(Chunk *chunk)
{
    chunk->len = 0;
    chunk->cap = 0;
    chunk->lines = NULL;
    chunk->code = NULL;
    val_list_init(&chunk->vals);
}

void chunk_free(Chunk *chunk)
{
    free(chunk->code);
    chunk->code = NULL;

    free(chunk->lines);
    chunk->lines = NULL;

    val_list_free(&chunk->vals);
}

void chunk_write(Chunk *chunk, uint8_t byte, int line)
{
    if (byte > UINT8_MAX)
    {
        fprintf(stderr, "Error: Instruction value %d exceeds maximum (%d)\n", byte, UINT8_MAX);
        exit(EXIT_FAILURE);
    }

    if (chunk->len + 1 > chunk->cap)
    {
        chunk->cap = chunk->cap < 8 ? 8 : chunk->cap * 2;

        chunk->code = realloc(chunk->code, chunk->cap * sizeof(inst_t));
        if (chunk->code == NULL)
        {
            fprintf(stderr, "failed to allocate memory for chunk\n");
            exit(EXIT_FAILURE);
        }

        chunk->lines = realloc(chunk->lines, chunk->cap * sizeof(int));
        if (chunk->lines == NULL)
        {
            fprintf(stderr, "failed to allocate memory for line nums\n");
            exit(EXIT_FAILURE);
        }
    }

    chunk->code[chunk->len] = byte;
    chunk->lines[chunk->len] = line;
    chunk->len++;
}

int chunk_val_add(Chunk *chunk, val_t val)
{
    val_list_push(&chunk->vals, val);
    return chunk->vals.len - 1;
}

int chunk_dissassemble_instruction(Chunk *chunk, int offset)
{
    printf("%04d | %04d ", offset, chunk->lines[offset]);

    switch (chunk->code[offset])
    {
    case OP_ERR:
        printf("OP_ERR ");
        break;
    case OP_VAL:
        printf("OP_VAL ");
        val_t op_val_val = chunk->vals.vals[chunk->code[offset + 1]];
        printf("%lx ", op_val_val);

        switch(val_kind(op_val_val))
        {
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_BOOL:
            printf("BLN %s", val_as_bool(op_val_val) ? "true" : "false");
            break;
        case VAL_PTR:
            printf("PTR %p", val_as_ptr(op_val_val));
            break;
        case VAL_OBJ:
            printf("OBJ %p", val_as_obj(op_val_val));
            break;
        case VAL_U8:
            printf("U8  %u", val_as_u8(op_val_val));
            break;
        case VAL_U16:
            printf("U16 %u", val_as_u16(op_val_val));
            break;
        case VAL_U32:
            printf("U32 %u", val_as_u32(op_val_val));
            break;
        case VAL_U64:
            printf("U64 %lu", val_as_u64(op_val_val));
            break;
        case VAL_I8:
            printf("I8  %d", val_as_i8(op_val_val));
            break;
        case VAL_I16:
            printf("I16 %d", val_as_i16(op_val_val));
            break;
        case VAL_I32:
            printf("I32 %d", val_as_i32(op_val_val));
            break;
        case VAL_I64:
            printf("I64 %ld", val_as_i64(op_val_val));
            break;
        case VAL_F32:
            printf("F32 %f", val_as_f32(op_val_val));
            break;
        case VAL_F64:
            printf("F64 %lf", val_as_f64(op_val_val));
            break;
        }

        printf("\n");

        return offset + 2;
    case OP_ADD:
        printf("OP_ADD ");
        break;
    case OP_SUB:
        printf("OP_SUB ");
        break;
    case OP_MUL:
        printf("OP_MUL ");
        break;
    case OP_DIV:
        printf("OP_DIV ");
        break;
    case OP_MOD:
        printf("OP_MOD ");
        break;
    case OP_NEG:
        printf("OP_NEG ");
        break;
    case OP_B_AND:
        printf("OP_B_AND ");
        break;
    case OP_B_OR:
        printf("OP_B_OR ");
        break;
    case OP_B_XOR:
        printf("OP_B_XOR ");
        break;
    case OP_B_NOT:
        printf("OP_B_NOT ");
        break;
    case OP_B_SHL:
        printf("OP_B_SHL ");
        break;
    case OP_B_SHR:
        printf("OP_B_SHR ");
        break;
    case OP_EQ:
        printf("OP_EQ ");
        break;
    case OP_NEQ:
        printf("OP_NEQ ");
        break;
    case OP_GT:
        printf("OP_GT ");
        break;
    case OP_GTE:
        printf("OP_GTE ");
        break;
    case OP_LT:
        printf("OP_LT ");
        break;
    case OP_LTE:
        printf("OP_LTE ");
        break;
    case OP_L_AND:
        printf("OP_L_AND ");
        break;
    case OP_L_OR:
        printf("OP_L_OR ");
        break;
    case OP_L_NOT:
        printf("OP_L_NOT ");
        break;
    case OP_JMP:
        printf("OP_JMP ");
        break;
    case OP_JNE:
        printf("OP_JNE ");
        break;
    case OP_RET:
        printf("OP_RET ");
        break;
    default:
        printf("UNKNOWN\n");
        break;
    }

    printf("\n");

    return offset + 1;
}

void chunk_dissassemble(Chunk *chunk, char *name)
{
    printf("%s:\n", name);
    for (int offset = 0; offset < chunk->len;)
    {
        offset = chunk_dissassemble_instruction(chunk, offset);
    }
    printf("\n");
}

void vm_init(VM *vm)
{
    vm->chunk = NULL;
    vm->ip = NULL;
    vm_stack_reset(vm);
}

void vm_free(VM *vm)
{
    vm->chunk = NULL;
    vm->ip = NULL;
}

void vm_stack_push(VM *vm, val_t val) {
    if (vm->stack_top - vm->stack + 1 > STACK_MAX)
    {
        fprintf(stderr, "stack overflow\n");
        exit(EXIT_FAILURE);
    }
    *vm->stack_top++ = val;
}

val_t vm_stack_pop(VM *vm) {
    if (vm->stack_top == vm->stack)
    {
        fprintf(stderr, "stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return *--vm->stack_top;
}

void vm_stack_reset(VM *vm)
{
    vm->stack_top = vm->stack;
}

VMRunResult vm_run(VM *vm, Chunk *chunk)
{
    for (;;)
    {
        inst_t inst;
        switch (inst = vm_ip_next(vm))
        {
        case OP_ERR:
            return VM_RUN_ERR_COMPILE;
        case OP_VAL:
            vm_stack_push(vm, chunk->vals.vals[vm_ip_next(vm)]);
            break;
        case OP_NEG:
            switch (val_kind(vm->stack_top[-1]))
            {
            case VAL_I8:
                vm_stack_push(vm, val_new_i8(-val_as_i8(vm_stack_pop(vm))));
                break;
            case VAL_I16:
                vm_stack_push(vm, val_new_i16(-val_as_i16(vm_stack_pop(vm))));
                break;
            case VAL_I32:
                vm_stack_push(vm, val_new_i32(-val_as_i32(vm_stack_pop(vm))));
                break;
            case VAL_I64:
                vm_stack_push(vm, val_new_i64(-val_as_i64(vm_stack_pop(vm))));
                break;
            case VAL_F32:
                vm_stack_push(vm, val_new_f32(-val_as_f32(vm_stack_pop(vm))));
                break;
            case VAL_F64:
                vm_stack_push(vm, val_new_f64(-val_as_f64(vm_stack_pop(vm))));
                break;
            default:
                fprintf(stderr, "invalid type for negation: %s\n", val_kind_str(val_kind(vm->stack_top[-1])));
                return VM_RUN_ERR_RUNTIME;
            }
            printf("negation of type %s: %d\n", val_kind_str(val_kind(vm->stack_top[-1])), val_as_i8(vm->stack_top[-1]));
            break;
        case OP_RET:
            return VM_RUN_OK;
        }
    }
}

int main(int arc, const char *argv[])
{
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_nil()), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_bool(true)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_ptr(&chunk)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_obj(&chunk)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_i8(0xF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_i16(0xFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_i32(0xFFFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_i64(0xFFFFFFFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_u8(0xF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_u16(0xFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_u32(0xFFFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_u64(0xFFFFFFFF)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_f32(FLT_MAX)), 1);
    chunk_write(&chunk, OP_VAL, 1);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_f64(DBL_MAX)), 1);

    chunk_write(&chunk, OP_NEG, 2);
    chunk_write(&chunk, OP_VAL, 2);
    chunk_write(&chunk, chunk_val_add(&chunk, val_new_u8(1)), 2);

    chunk_write(&chunk, OP_RET, 99);

    printf("offs | line\n");
    chunk_dissassemble(&chunk, "main");

    printf("EXEC:\n");
    VM vm;
    vm_init(&vm);
    vm.chunk = &chunk;
    vm.ip = chunk.code;

    VMRunResult result = vm_run(&vm, &chunk);
    printf("---\n");
    if (result == VM_RUN_OK)
        printf("OK\n");
    else
        printf("ERR\n");
    vm_free(&vm);

    chunk_free(&chunk);

    return 0;
}
