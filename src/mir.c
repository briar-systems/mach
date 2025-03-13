#include "mir.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool mir_context_init(MIRContext *context)
{
    if (!context)
    {
        return false;
    }

    context->modules = NULL;
    context->module_count = 0;

    context->string_constants = NULL;
    context->string_constant_count = 0;

    return true;
}

void mir_context_free(MIRContext *context)
{
    if (!context)
    {
        return;
    }

    for (int i = 0; i < context->module_count; i++)
    {
        mir_module_free(context->modules[i]);
        context->modules[i] = NULL;
    }
    free(context->modules);
    context->modules = NULL;

    for (int i = 0; i < context->string_constant_count; i++)
    {
        free(context->string_constants[i]);
        context->string_constants[i] = NULL;
    }
    free(context->string_constants);
    context->string_constants = NULL;

    free(context);
}

bool mir_context_add(MIRContext *context, MIRModule *module)
{
    if (!context || !module)
    {
        return false;
    }

    context->module_count++;
    context->modules = realloc(context->modules, context->module_count * sizeof(MIRModule *));
    if (!context->modules)
    {
        return false;
    }

    context->modules[context->module_count - 1] = module;

    return true;
}

bool mir_module_init(MIRModule *module, MIRContext *context, const char *name, Target target)
{
    if (!module || !context || !name)
    {
        return false;
    }

    module->context = context;
    module->target = target;

    module->name = strdup(name);
    if (!module->name)
    {
        return false;
    }

    module->functions = NULL;
    module->function_count = 0;

    if (!mir_context_add(context, module))
    {
        free(module->name);
        return false;
    }

    return true;
}

void mir_module_free(MIRModule *module)
{
    if (!module)
    {
        return;
    }

    for (int i = 0; i < module->function_count; i++)
    {
        mir_function_free(module->functions[i]);
        module->functions[i] = NULL;
    }
    free(module->functions);
    module->functions = NULL;

    free(module->name);
    module->name = NULL;

    free(module);
}

bool mir_module_add_function(MIRModule *module, MIRFunction *function)
{
    if (!module || !function)
    {
        return false;
    }

    module->function_count++;
    module->functions = realloc(module->functions, module->function_count * sizeof(MIRFunction *));
    if (!module->functions)
    {
        return false;
    }

    module->functions[module->function_count - 1] = function;

    return true;
}

bool mir_function_init(MIRFunction *function, MIRModule *module, const char *name, Type *type)
{
    if (!module || !name || !type || type->kind != TYPE_FUN)
    {
        return false;
    }

    function->name = strdup(name);
    if (!function->name)
    {
        return false;
    }

    function->type = type;
    function->parent = module;

    if (!mir_module_add_function(module, function))
    {
        free(function->name);
        free(function);
        return false;
    }

    MIRBlock *block = calloc(1, sizeof(MIRBlock));
    if (!mir_block_init(block, function, "entry"))
    {
        free(function->name);
        free(function);
        return false;
    }
    block->is_entry = true;

    function->entry_block = block;

    if (type->fun.param_count > 0)
    {
        function->parameters = calloc(type->fun.param_count, sizeof(MIRValue));
        if (!function->parameters)
        {
            free(function->name);
            free(function);
            return false;
        }

        function->parameter_count = type->fun.param_count;
    }

    return true;
}

void mir_function_free(MIRFunction *function)
{
    if (!function)
    {
        return;
    }

    for (int i = 0; i < function->block_count; i++)
    {
        mir_block_free(function->blocks[i]);
        function->blocks[i] = NULL;
    }
    free(function->blocks);
    function->blocks = NULL;

    free(function->parameters);
    function->parameters = NULL;

    free(function->name);
    function->name = NULL;

    free(function);
}

MIRValue mir_function_add_parameter(MIRFunction *function, Type *type)
{
    if (!function || !type)
    {
        return (MIRValue){.kind = MIR_VAL_ERROR};
    }

    int param_index = 0;
    while (param_index < function->parameter_count && function->parameters[param_index].kind != MIR_VAL_NONE)
    {
        param_index++;
    }

    if (param_index >= function->parameter_count)
    {
        return (MIRValue){.kind = MIR_VAL_ERROR};
    }

    MIRValue param = mir_create_register(function, type);
    if (param.kind == MIR_VAL_ERROR)
    {
        return param;
    }

    function->parameters[param_index] = param;

    return param;
}

bool mir_function_add_block(MIRFunction *function, MIRBlock *block)
{
    if (!function || !block)
    {
        return false;
    }

    function->block_count++;
    function->blocks = realloc(function->blocks, function->block_count * sizeof(MIRBlock *));
    if (!function->blocks)
    {
        return false;
    }

    function->blocks[function->block_count - 1] = block;

    return true;
}

bool mir_block_init(MIRBlock *block, MIRFunction *function, const char *label)
{
    if (!block || !function)
    {
        return false;
    }

    block->parent = function;
    block->predecessors = NULL;
    block->successors = NULL;

    block->predecessor_count = 0;
    block->successor_count = 0;

    block->first = NULL;
    block->last = NULL;

    block->label = strdup(label);

    if (!mir_function_add_block(function, block))
    {
        return false;
    }

    return true;
}

void mir_block_free(MIRBlock *block)
{
    if (!block)
    {
        return;
    }

    for (int i = 0; i < block->predecessor_count; i++)
    {
        free(block->predecessors[i]);
        block->predecessors[i] = NULL;
    }
    free(block->predecessors);
    block->predecessors = NULL;

    for (int i = 0; i < block->successor_count; i++)
    {
        free(block->successors[i]);
        block->successors[i] = NULL;
    }
    free(block->successors);
    block->successors = NULL;

    free(block->label);
    block->label = NULL;

    free(block);
}

bool mir_block_add_successor(MIRBlock *block, MIRBlock *successor)
{
    if (!block || !successor)
    {
        return false;
    }

    for (int i = 0; i < block->successor_count; i++)
    {
        if (block->successors[i] == successor)
        {
            return false;
        }
    }

    if (!mir_block_add_predecessor(successor, block))
    {
        return false;
    }

    block->successor_count++;
    block->successors = realloc(block->successors, block->successor_count * sizeof(MIRBlock *));
    if (!block->successors)
    {
        return false;
    }

    block->successors[block->successor_count - 1] = successor;

    return true;
}

bool mir_block_add_predecessor(MIRBlock *block, MIRBlock *predecessor)
{
    if (!block || !predecessor)
    {
        return false;
    }

    for (int i = 0; i < block->predecessor_count; i++)
    {
        if (block->predecessors[i] == predecessor)
        {
            return false;
        }
    }

    block->predecessor_count++;
    block->predecessors = realloc(block->predecessors, block->predecessor_count * sizeof(MIRBlock *));
    if (!block->predecessors)
    {
        return false;
    }

    block->predecessors[block->predecessor_count - 1] = predecessor;

    return true;
}

bool mir_inst_init(MIRInst *inst, MIRBlock *block, MIROpcode opcode)
{
    if (!inst || !block)
    {
        return false;
    }

    inst->opcode = opcode;
    inst->parent = block;

    if (block->first == NULL)
    {
        block->first = inst;
        block->last = inst;
    }
    else
    {
        inst->prev = block->last;
        block->last->next = inst;
        block->last = inst;
    }

    return true;
}

void mir_inst_free(MIRInst *inst)
{
    if (!inst)
    {
        return;
    }

    free(inst->operands);
    inst->operands = NULL;

    free(inst);
}

void mir_inst_remove(MIRInst *inst)
{
    if (!inst)
    {
        return;
    }

    if (inst->parent)
    {
        if (inst->prev)
        {
            inst->prev->next = inst->next;
        }
        else
        {
            inst->parent->first = inst->next;
        }

        if (inst->next)
        {
            inst->next->prev = inst->prev;
        }
        else
        {
            inst->parent->last = inst->prev;
        }
    }

    inst->prev = NULL;
    inst->next = NULL;
    inst->parent = NULL;
}

void mir_inst_insert_after(MIRInst *inst, MIRInst *after)
{
    if (!inst || !after || inst == after)
    {
        return;
    }

    mir_inst_remove(inst);

    MIRInst *next = after->next;
    after->next = inst;
    inst->prev = after;
    inst->next = next;

    if (next)
    {
        next->prev = inst;
    }
    else if (after->parent)
    {
        after->parent->last = inst;
    }

    inst->parent = after->parent;
}

void mir_inst_insert_before(MIRInst *inst, MIRInst *before)
{
    if (!inst || !before || inst == before)
    {
        return;
    }

    mir_inst_remove(inst);

    MIRInst *prev = before->prev;
    before->prev = inst;
    inst->next = before;
    inst->prev = prev;

    if (prev)
    {
        prev->next = inst;
    }
    else if (before->parent)
    {
        before->parent->first = inst;
    }

    inst->parent = before->parent;
}

bool mir_inst_add_operand(MIRInst *inst, MIRValue operand)
{
    if (!inst)
    {
        return false;
    }

    inst->operand_count++;
    inst->operands = realloc(inst->operands, inst->operand_count * sizeof(MIRValue));
    if (!inst->operands)
    {
        return false;
    }

    inst->operands[inst->operand_count - 1] = operand;

    return true;
}

MIRValue mir_value_error(const char *message)
{
    MIRValue value;
    value.kind = MIR_VAL_ERROR;
    value.err.message = strdup(message);

    if (!value.err.message)
    {
        value.err.message = "mir_value_error <memory error>";
    }

    return value;
}

MIRValue mir_value_register(Type *type, unsigned int id)
{
    MIRValue value;
    value.kind = MIR_VAL_REGISTER;
    value.type = type;
    value.reg.id = id;
    return value;
}

MIRValue mir_value_global(Type *type, const char *name)
{
    MIRValue value;
    value.kind = MIR_VAL_GLOBAL;
    value.type = type;
    value.global.name = strdup(name);
    if (!value.global.name)
    {
        return mir_value_error("mir_value_global: value.global.name: memory error");
    }

    return value;
}

MIRValue mir_value_block(MIRBlock *block)
{
    MIRValue value;
    value.kind = MIR_VAL_BLOCK;
    value.type = NULL;
    value.block = block;
    return value;
}

MIRValue mir_value_function(MIRFunction *function)
{
    MIRValue value;
    value.kind = MIR_VAL_FUNCTION;
    value.type = function->type;
    value.function = function;
    return value;
}

MIRValue mir_const_int(Type *type, long long value)
{
    MIRValue val;
    val.kind = MIR_VAL_CONSTANT;
    val.type = type;
    val.constant.i = value;
    return val;
}

MIRValue mir_const_float(Type *type, double value)
{
    MIRValue val;
    val.kind = MIR_VAL_CONSTANT;
    val.type = type;
    val.constant.f = value;
    return val;
}

MIRValue mir_const_ptr(Type *type, void *value)
{
    MIRValue val;
    val.kind = MIR_VAL_CONSTANT;
    val.type = type;
    val.constant.i = (long long)value;
    return val;
}

MIRValue mir_const_str(MIRContext *context, const char *value)
{
    if (!context || !value)
    {
        return mir_value_error("mir_const_str: context or value is NULL");
    }

    context->string_constant_count++;
    context->string_constants = realloc(context->string_constants, context->string_constant_count * sizeof(char *));
    if (!context->string_constants)
    {
        return mir_value_error("mir_const_str: context->string_constants: memory error");
    }

    int string_index = context->string_constant_count - 1;
    context->string_constants[string_index] = strdup(value);
    if (!context->string_constants[string_index])
    {
        return mir_value_error("mir_const_str: context->string_constants[string_index]: memory error");
    }

    // strings are pointers to char arrays, e.g `#[]u8` or `char*`
    Type *type = calloc(sizeof(Type), 1);
    if (!type)
    {
        return mir_value_error("mir_const_str: type: memory error");
    }
    type->kind = TYPE_PTR;

    type->ptr.target = calloc(sizeof(Type), 1);
    if (!type->ptr.target)
    {
        free(type);
        return mir_value_error("mir_const_str: type->ptr.target: memory error");
    }
    type->ptr.target->kind = TYPE_ARR;
    type->ptr.target->arr.len = strlen(value);

    type->ptr.target->arr.element_type = calloc(sizeof(Type), 1);
    if (!type->ptr.target->arr.element_type)
    {
        free(type->ptr.target);
        free(type);
        return mir_value_error("mir_const_str: type->ptr.target->arr.element_type: memory error");
    }

    if (!type_init(type->ptr.target->arr.element_type, TYPE_U8))
    {
        free(type->ptr.target->arr.element_type);
        free(type->ptr.target);
        free(type);
        return mir_value_error("mir_const_str: type->ptr.target->arr.element_type: memory error");
    }

    MIRValue val;
    val.kind = MIR_VAL_CONSTANT;
    val.type = type;
    val.constant.s = context->string_constants[string_index];
    return val;
}

MIRValue mir_create_register(MIRFunction *function, Type *type)
{
    if (!function || !type)
    {
        return mir_value_error("mir_create_register: function or type is NULL");
    }

    unsigned int reg_id = function->reg_count++;

    return mir_value_register(type, reg_id);
}

MIRValue mir_build_alloca(MIRBlock *block, Type *type)
{
    if (!block || !type)
    {
        return mir_value_error("mir_build_alloca: block or type is NULL");
    }

    Type *ptr_type = type_make(TYPE_PTR);
    if (!ptr_type)
    {
        return mir_value_error("mir_build_alloca: type_make failed");
    }
    ptr_type->ptr.target = type;

    MIRValue result = mir_create_register(block->parent, ptr_type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_ALLOCA))
    {
        free(inst);
        return mir_value_error("mir_build_alloca: mir_inst_init failed");
    }
    inst->result = result;

    MIRValue type_operand = {.kind = MIR_VAL_NONE, .type = type};
    if (!mir_inst_add_operand(inst, type_operand))
    {
        free(inst);
        return mir_value_error("mir_build_alloca: mir_inst_add_operand failed");
    }

    return result;
}

MIRValue mir_build_load(MIRBlock *block, MIRValue ptr)
{
    if (!block || ptr.kind == MIR_VAL_NONE || !ptr.type || ptr.type->kind != TYPE_PTR)
    {
        return mir_value_error("mir_build_load: block or ptr is NULL");
    }

    Type *value_type = ptr.type->ptr.target;

    MIRValue result = mir_create_register(block->parent, value_type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_LOAD))
    {
        free(inst);
        return mir_value_error("mir_build_load: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, ptr))
    {
        free(inst);
        return mir_value_error("mir_build_load: mir_inst_add_operand failed");
    }

    return result;
}

bool mir_build_store(MIRBlock *block, MIRValue ptr, MIRValue value)
{
    if (!block || ptr.kind == MIR_VAL_NONE || value.kind == MIR_VAL_NONE || !ptr.type || !value.type || ptr.type->kind != TYPE_PTR)
    {
        return false;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_STORE))
    {
        free(inst);
        return false;
    }

    if (!mir_inst_add_operand(inst, ptr) || !mir_inst_add_operand(inst, value))
    {
        free(inst);
        return false;
    }

    return true;
}

MIRValue mir_build_offset(MIRBlock *block, MIRValue base, MIRValue index, size_t element_size)
{
    if (!block || base.kind == MIR_VAL_NONE || index.kind == MIR_VAL_NONE || !base.type || base.type->kind != TYPE_PTR)
    {
        return mir_value_error("mir_build_offset: block or base is NULL");
    }

    MIRValue result = mir_create_register(block->parent, base.type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_OFFSET))
    {
        free(inst);
        return mir_value_error("mir_build_offset: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, base) || !mir_inst_add_operand(inst, index))
    {
        free(inst);
        return mir_value_error("mir_build_offset: mir_inst_add_operand failed");
    }

    MIRValue size_operand = mir_const_int(NULL, (long long)element_size);
    if (size_operand.kind == MIR_VAL_ERROR)
    {
        free(inst);
        return size_operand;
    }

    if (!mir_inst_add_operand(inst, size_operand))
    {
        free(inst);
        return mir_value_error("mir_build_offset: mir_inst_add_operand failed");
    }

    return result;
}

MIRValue mir_build_field_offset(MIRBlock *block, MIRValue base, int field_index)
{
    if (!block || base.kind == MIR_VAL_NONE || !base.type || base.type->kind != TYPE_PTR)
    {
        return mir_value_error("mir_build_field_offset: block or base is NULL");
    }

    Type *struct_type = base.type->ptr.target;
    if (struct_type->kind != TYPE_STR)
    {
        return mir_value_error("mir_build_field_offset: base is not a pointer to a struct");
    }

    // create a virtual register for the result
    Type *field_type = struct_type->str.field_types[field_index];
    Type *field_ptr_type = type_make(TYPE_PTR);
    if (!field_ptr_type)
    {
        return mir_value_error("mir_build_field_offset: type_make failed");
    }
    field_ptr_type->ptr.target = field_type;

    MIRValue result = mir_create_register(block->parent, field_ptr_type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_OFFSET))
    {
        free(inst);
        return mir_value_error("mir_build_field_offset: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, base))
    {
        free(inst);
        return mir_value_error("mir_build_field_offset: mir_inst_add_operand failed");
    }

    size_t offset = 0;
    if (struct_type->str.field_offsets)
    {
        offset = struct_type->str.field_offsets[field_index];
    }
    else
    {
        for (int i = 0; i < field_index; i++)
        {
            Type *type = struct_type->str.field_types[i];
            if (!type)
            {
                free(inst);
                return mir_value_error("mir_build_field_offset: struct field type is NULL");
            }

            size_t field_size = type_size(type, block->parent->parent->target);
            size_t field_align = type_align(type, block->parent->parent->target);

            offset = (offset + field_align - 1) & ~(field_align - 1);
            offset += field_size;
        }
    }

    MIRValue offset_operand = mir_const_int(NULL, offset);
    if (offset_operand.kind == MIR_VAL_ERROR)
    {
        free(inst);
        return offset_operand;
    }

    if (!mir_inst_add_operand(inst, offset_operand))
    {
        free(inst);
        return mir_value_error("mir_build_field_offset: mir_inst_add_operand failed");
    }

    return result;
}

MIRValue mir_build_binary(MIRBlock *block, MIROpcode opcode, MIRValue left, MIRValue right)
{
    if (!block || left.kind == MIR_VAL_NONE || right.kind == MIR_VAL_NONE || !left.type || !right.type)
    {
        return mir_value_error("mir_build_binary: block, left, or right is NULL");
    }

    switch (opcode)
    {
    case MIR_ADD:
    case MIR_SUB:
    case MIR_MUL:
    case MIR_DIV:
    case MIR_MOD:
    case MIR_AND:
    case MIR_OR:
    case MIR_XOR:
    case MIR_SHL:
    case MIR_SHR:
    case MIR_EQ:
    case MIR_NE:
    case MIR_LT:
    case MIR_LE:
    case MIR_GT:
    case MIR_GE:
        break;
    default:
        return mir_value_error("mir_build_binary: invalid binary opcode");
    }

    Type *result_type;

    if (opcode == MIR_EQ || opcode == MIR_NE || opcode == MIR_LT || opcode == MIR_LE || opcode == MIR_GT || opcode == MIR_GE)
    {
        result_type = calloc(1, sizeof(Type));
        if (!type_init(result_type, TYPE_U8))
        {
            fprintf(stderr, "error: failed to create boolean type\n");
            return mir_value_error("mir_build_binary: failed to create boolean type");
        }
    }
    else
    {
        result_type = left.type;
    }

    MIRValue result = mir_create_register(block->parent, result_type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, opcode))
    {
        free(inst);
        return mir_value_error("mir_build_binary: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, left) || !mir_inst_add_operand(inst, right))
    {
        free(inst);
        return mir_value_error("mir_build_binary: mir_inst_add_operand failed");
    }

    return result;
}

MIRValue mir_build_unary(MIRBlock *block, MIROpcode opcode, MIRValue operand)
{
    if (!block || operand.kind == MIR_VAL_NONE || !operand.type)
    {
        return mir_value_error("mir_build_unary: block or operand is NULL");
    }

    switch (opcode)
    {
    case MIR_NEG:
    case MIR_NOT:
        break;
    default:
        return mir_value_error("mir_build_unary: invalid unary opcode");
    }

    MIRValue result = mir_create_register(block->parent, operand.type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, opcode))
    {
        free(inst);
        return mir_value_error("mir_build_unary: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, operand))
    {
        free(inst);
        return mir_value_error("mir_build_unary: mir_inst_add_operand failed");
    }

    return result;
}

bool mir_build_jmp(MIRBlock *block, MIRBlock *target)
{
    if (!block || !target)
    {
        return false;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_JMP))
    {
        free(inst);
        return false;
    }

    MIRValue target_value = mir_value_block(target);
    if (!mir_inst_add_operand(inst, target_value))
    {
        free(inst);
        return false;
    }

    if (!mir_inst_add_operand(inst, target_value))
    {
        free(inst);
        return false;
    }

    if (!mir_block_add_successor(block, target))
    {
        free(inst);
        return false;
    }

    return true;
}

bool mir_build_cjmp(MIRBlock *block, MIRValue condition, MIRBlock *true_target, MIRBlock *false_target)
{
    if (!block || condition.kind == MIR_VAL_NONE || !true_target || !false_target)
    {
        return false;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_CJMP))
    {
        free(inst);
        return false;
    }

    MIRValue true_target_value = mir_value_block(true_target);
    MIRValue false_target_value = mir_value_block(false_target);
    if (!mir_inst_add_operand(inst, condition) || !mir_inst_add_operand(inst, true_target_value) || !mir_inst_add_operand(inst, false_target_value))
    {
        free(inst);
        return false;
    }

    if (!mir_inst_add_operand(inst, condition))
    {
        free(inst);
        return false;
    }

    if (!mir_inst_add_operand(inst, true_target_value))
    {
        free(inst);
        return false;
    }

    if (!mir_inst_add_operand(inst, false_target_value))
    {
        free(inst);
        return false;
    }

    if (!mir_block_add_successor(block, true_target) || !mir_block_add_successor(block, false_target))
    {
        free(inst);
        return false;
    }
    
    return true;
}

bool mir_build_ret(MIRBlock *block, MIRValue value)
{
    if (!block)
    {
        return false;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_RET))
    {
        free(inst);
        return false;
    }

    if (value.kind != MIR_VAL_NONE)
    {
        if (!mir_inst_add_operand(inst, value))
        {
            free(inst);
            return false;
        }
    }

    block->is_exit = true;

    return true;
}

MIRValue mir_build_call(MIRBlock *block, MIRValue function, MIRValue *args, int arg_count)
{
    if (!block || function.kind == MIR_VAL_NONE || !function.type || function.type->kind != TYPE_FUN)
    {
        return mir_value_error("mir_build_call: block or function is NULL");
    }

    MIRValue result = {.kind = MIR_VAL_NONE};
    Type *return_type = function.type->fun.return_type;
    if (return_type->kind != TYPE_VOID)
    {
        result = mir_create_register(block->parent, return_type);
        if (result.kind == MIR_VAL_ERROR)
        {
            return result;
        }
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_CALL))
    {
        free(inst);
        return mir_value_error("mir_build_call: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, function))
    {
        free(inst);
        return mir_value_error("mir_build_call: mir_inst_add_operand failed");
    }

    for (int i = 0; i < arg_count; i++)
    {
        if (args[i].kind != MIR_VAL_NONE)
        {
            if (!mir_inst_add_operand(inst, args[i]))
            {
                free(inst);
                return mir_value_error("mir_build_call: mir_inst_add_operand failed");
            }
        }
    }

    return result;
}

MIRValue mir_build_cast(MIRBlock *block, MIRValue value, Type *target_type)
{
    if (!block || value.kind == MIR_VAL_NONE || !value.type || !target_type)
    {
        return mir_value_error("mir_build_cast: block, value, or target_type is NULL");
    }

    MIRValue result = mir_create_register(block->parent, target_type);
    if (result.kind == MIR_VAL_ERROR)
    {
        return result;
    }

    MIRInst *inst = calloc(sizeof(MIRInst), 1);
    if (!mir_inst_init(inst, block, MIR_CAST))
    {
        free(inst);
        return mir_value_error("mir_build_cast: mir_inst_init failed");
    }
    inst->result = result;

    if (!mir_inst_add_operand(inst, value))
    {
        free(inst);
        return mir_value_error("mir_build_cast: mir_inst_add_operand failed");
    }

    MIRValue type_operand = {.kind = MIR_VAL_NONE, .type = target_type};
    if (!mir_inst_add_operand(inst, type_operand))
    {
        free(inst);
        return mir_value_error("mir_build_cast: mir_inst_add_operand failed");
    }

    return result;
}

void mir_compute_cfg(MIRFunction *function)
{
    if (!function)
    {
        return;
    }

    for (int i = 0; i < function->block_count; i++)
    {
        MIRBlock *block = function->blocks[i];

        free(block->predecessors);
        block->predecessors = NULL;
        block->predecessor_count = 0;

        free(block->successors);
        block->successors = NULL;
        block->successor_count = 0;
    }

    for (int i = 0; i < function->block_count; i++)
    {
        MIRBlock *block = function->blocks[i];
        MIRInst *terminator = block->last;

        if (!terminator)
        {
            continue;
        }

        switch (terminator->opcode)
        {
        case MIR_JMP:
            if (terminator->operand_count >= 1 && terminator->operands[0].kind == MIR_VAL_BLOCK)
            {
                if (!mir_block_add_successor(block, terminator->operands[0].block))
                {
                    return;
                }
            }
            break;

        case MIR_CJMP:
            if (terminator->operand_count >= 3 && terminator->operands[1].kind == MIR_VAL_BLOCK && terminator->operands[2].kind == MIR_VAL_BLOCK)
            {
                if (!mir_block_add_successor(block, terminator->operands[1].block) || !mir_block_add_successor(block, terminator->operands[2].block))
                {
                    return;
                }
            }
            break;

        case MIR_RET:
            block->is_exit = true;
            break;

        default:
            if (i + 1 < function->block_count)
            {
                if (!mir_block_add_successor(block, function->blocks[i + 1]))
                {
                    return;
                }
            }
            break;
        }
    }
}

void mir_validate(MIRModule *module)
{
    if (!module)
    {
        return;
    }

    printf("Validating module '%s'...\n", module->name);

    for (int i = 0; i < module->function_count; i++)
    {
        MIRFunction *function = module->functions[i];

        if (!function->name || !function->name[0])
        {
            printf("  Warning: Function %d has no name\n", i);
        }

        if (!function->type || function->type->kind != TYPE_FUN)
        {
            printf("  error: Function '%s' has invalid type\n", function->name);
            continue;
        }

        if (!function->entry_block)
        {
            printf("  error: Function '%s' has no entry block\n", function->name);
            continue;
        }

        bool has_exit = false;
        for (int j = 0; j < function->block_count; j++)
        {
            MIRBlock *block = function->blocks[j];

            if (!block->last)
            {
                printf("  error: Block '%d' in function '%s' is empty\n", j, function->name);
                continue;
            }

            MIROpcode terminator = block->last->opcode;
            switch (terminator)
            {
            case MIR_JMP:
            case MIR_CJMP:
            case MIR_RET:
                break;

            default:
                printf("  error: Block '%d' in function '%s' does not end with a terminator\n", j, function->name);
                break;
            }

            if (block->is_exit)
            {
                has_exit = true;
            }
        }

        if (!has_exit)
        {
            printf("  error: Function '%s' has no exit blocks\n", function->name);
        }
    }

    printf("Validation complete.\n");
}

void mir_optimize(MIRModule *module)
{
    printf("No optimizations to be completed for module: %s\n", module->name);
}

const char *mir_opcode_name(MIROpcode opcode)
{
    switch (opcode)
    {
    case MIR_NOP:
        return "nop";
    case MIR_ALLOCA:
        return "alloca";
    case MIR_LOAD:
        return "load";
    case MIR_STORE:
        return "store";
    case MIR_OFFSET:
        return "offset";
    case MIR_ADD:
        return "add";
    case MIR_SUB:
        return "sub";
    case MIR_MUL:
        return "mul";
    case MIR_DIV:
        return "div";
    case MIR_MOD:
        return "mod";
    case MIR_NEG:
        return "neg";
    case MIR_AND:
        return "and";
    case MIR_OR:
        return "or";
    case MIR_XOR:
        return "xor";
    case MIR_SHL:
        return "shl";
    case MIR_SHR:
        return "shr";
    case MIR_NOT:
        return "not";
    case MIR_EQ:
        return "eq";
    case MIR_NE:
        return "ne";
    case MIR_LT:
        return "lt";
    case MIR_LE:
        return "le";
    case MIR_GT:
        return "gt";
    case MIR_GE:
        return "ge";
    case MIR_JMP:
        return "jmp";
    case MIR_CJMP:
        return "cjmp";
    case MIR_RET:
        return "ret";
    case MIR_CALL:
        return "call";
    case MIR_CAST:
        return "cast";
    case MIR_CONST_INT:
        return "const.int";
    case MIR_CONST_FLOAT:
        return "const.float";
    case MIR_CONST_PTR:
        return "const.ptr";
    case MIR_CONST_STR:
        return "const.str";
    case MIR_PHI:
        return "phi";
    default:
        return "unknown";
    }
}

void mir_print_value(MIRValue value)
{
    if (value.kind == MIR_VAL_NONE)
    {
        printf("<none>");
        return;
    }

    switch (value.kind)
    {
    case MIR_VAL_NONE:
        printf("<none>");
        break;
    case MIR_VAL_ERROR:
        printf("<error: %s>", value.err.message);
        break;
    case MIR_VAL_REGISTER:
        printf("%%r%d", value.reg.id);
        break;

    case MIR_VAL_GLOBAL:
        printf("@%s", value.global.name);
        break;

    case MIR_VAL_CONSTANT:
        if (value.type)
        {
            switch (value.type->kind)
            {
            case TYPE_F32:
            case TYPE_F64:
                printf("%f", value.constant.f);
                break;

            case TYPE_PTR:
                if (value.constant.i == 0)
                {
                    printf("null");
                }
                else
                {
                    printf("0x%llx", value.constant.i);
                }
                break;

            default:
                printf("%lld", value.constant.i);
                break;
            }
        }
        else
        {
            printf("%lld", value.constant.i);
        }
        break;

    case MIR_VAL_BLOCK:
        if (value.block)
        {
            printf("%%%s", value.block->label);
        }
        else
        {
            printf("<unknown block>");
        }
        break;

    case MIR_VAL_FUNCTION:
        if (value.function && value.function->name)
        {
            printf("@%s", value.function->name);
        }
        else
        {
            printf("<unknown function>");
        }
        break;

    default:
        printf("<unknown>");
        break;
    }
}

void mir_print_inst(MIRInst *inst)
{
    if (!inst)
    {
        return;
    }

    if (inst->result.kind != MIR_VAL_NONE)
    {
        mir_print_value(inst->result);
        printf(" = ");
    }

    printf("%s", mir_opcode_name(inst->opcode));

    if (inst->operand_count > 0)
    {
        printf(" ");

        for (int i = 0; i < inst->operand_count; i++)
        {
            if (i > 0)
            {
                printf(", ");
            }
            mir_print_value(inst->operands[i]);
        }
    }

    if (inst->result.kind != MIR_VAL_NONE && inst->result.type)
    {
        printf(" : ");
        char *type_str = type_to_string(inst->result.type);
        printf("%s", type_str);
        free(type_str);
    }

    printf("\n");
}

void mir_print_block(MIRBlock *block)
{
    if (!block)
    {
        return;
    }

    printf("%s:\n", block->label);

    if (block->predecessor_count > 0)
    {
        printf("  ; predecessors: ");
        for (int i = 0; i < block->predecessor_count; i++)
        {
            if (i > 0)
            {
                printf(", ");
            }
            printf("%%%s", block->predecessors[i]->label);
        }
        printf("\n");
    }

    MIRInst *inst = block->first;
    while (inst)
    {
        printf("  ");
        mir_print_inst(inst);
        inst = inst->next;
    }

    if (block->successor_count > 0)
    {
        printf("  ; successors: ");
        for (int i = 0; i < block->successor_count; i++)
        {
            if (i > 0)
            {
                printf(", ");
            }
            printf("%%%s", block->successors[i]->label);
        }
        printf("\n");
    }
}

void mir_print_function(MIRFunction *function)
{
    if (!function)
        return;

    printf("\nfunction %s", function->name);

    printf("(");
    for (int i = 0; i < function->parameter_count; i++)
    {
        if (i > 0)
        {
            printf(", ");
        }

        mir_print_value(function->parameters[i]);

        if (function->parameters[i].type)
        {
            printf(": ");
            char *type_str = type_to_string(function->parameters[i].type);
            printf("%s", type_str);
            free(type_str);
        }
    }
    printf(")");

    if (function->type && function->type->fun.return_type)
    {
        printf(" -> ");
        char *type_str = type_to_string(function->type->fun.return_type);
        printf("%s", type_str);
        free(type_str);
    }

    printf(" {\n");

    for (int i = 0; i < function->block_count; i++)
    {
        mir_print_block(function->blocks[i]);

        if (i < function->block_count - 1)
        {
            printf("\n");
        }
    }

    printf("}\n");
}

void mir_print_module(MIRModule *module)
{
    if (!module)
        return;

    printf("module %s {\n", module->name);

    TargetInfo t_info = target_info(module->target);
    printf("  target: %s-%s-%s\n", platform_to_string(module->target.platform), architecture_to_string(module->target.architecture), t_info.endian == ENDIAN_LITTLE ? "little" : "big");

    for (int i = 0; i < module->function_count; i++)
    {
        mir_print_function(module->functions[i]);
    }

    printf("}\n");
}

void mir_dump_dot(MIRFunction *function, const char *filename)
{
    if (!function || !filename)
    {
        return;
    }

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        return;
    }

    fprintf(file, "digraph \"%s\" {\n", function->name);
    fprintf(file, "  label=\"Function: %s\";\n", function->name);
    fprintf(file, "  node [shape=box];\n");

    for (int i = 0; i < function->block_count; i++)
    {
        MIRBlock *block = function->blocks[i];

        fprintf(file, "  \"%s\" [", block->label);

        if (block->is_entry)
        {
            fprintf(file, "style=filled, fillcolor=lightblue, ");
        }
        else if (block->is_exit)
        {
            fprintf(file, "style=filled, fillcolor=lightgrey, ");
        }

        fprintf(file, "label=\"%s:\\n", block->label);

        MIRInst *inst = block->first;
        while (inst)
        {
            char *opcode = strdup(mir_opcode_name(inst->opcode));
            for (char *c = opcode; *c; c++)
            {
                if (*c == '\"')
                    *c = '\'';
            }

            fprintf(file, "  %s", opcode);
            free(opcode);

            // Add operand count
            fprintf(file, " (ops: %d)\\n", inst->operand_count);

            inst = inst->next;
        }

        fprintf(file, "\"];\n");
    }

    for (int i = 0; i < function->block_count; i++)
    {
        MIRBlock *block = function->blocks[i];

        for (int j = 0; j < block->successor_count; j++)
        {
            MIRBlock *succ = block->successors[j];

            MIRInst *terminator = block->last;
            if (terminator && terminator->opcode == MIR_CJMP && j < 2)
            {
                if (j == 0)
                {
                    fprintf(file, "  \"%s\" -> \"%s\" [label=\"true\", color=green];\n", block->label, succ->label);
                }
                else
                {
                    fprintf(file, "  \"%s\" -> \"%s\" [label=\"false\", color=red];\n", block->label, succ->label);
                }
            }
            else
            {
                fprintf(file, "  \"%s\" -> \"%s\";\n", block->label, succ->label);
            }
        }
    }

    fprintf(file, "}\n");

    fclose(file);
    printf("Control flow graph written to '%s'\n", filename);
}
