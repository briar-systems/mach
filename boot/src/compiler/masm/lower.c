#include "compiler/masm/lower.h"
#include "compiler/masm/instruction.h"
#include "compiler/masm/isa/x86_64.h" // default to x86_64 for now
#include <stdio.h>
#include <string.h>

// helper to lower a function
static void lower_stmt(Masm *masm, MasmSection *text, AstNode *stmt);
static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr);

static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr)
{
    (void)masm;
    (void)text;
    
    if (expr->kind == AST_EXPR_LIT)
    {
        if (expr->lit_expr.kind == TOKEN_LIT_INT)
        {
            return masm_operand_imm((int64_t)expr->lit_expr.int_val);
        }
    }
    
    return masm_operand_none();
}

static void lower_stmt(Masm *masm, MasmSection *text, AstNode *stmt)
{
    if (stmt->kind == AST_STMT_RET)
    {
        AstNode *expr = stmt->ret_stmt.expr;
        if (expr)
        {
            MasmOperand op = lower_expr(masm, text, expr);
            // move result to rax (hardcoded for now, should use ABI)
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, masm_operand_register(MASM_X86_RAX, 8), op));
        }
        masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    }
    else if (stmt->kind == AST_STMT_BLOCK)
    {
        AstList *stmts = stmt->block_stmt.stmts;
        for (int i = 0; i < stmts->count; i++)
        {
            lower_stmt(masm, text, stmts->items[i]);
        }
    }
}

static void lower_function(Masm *masm, AstNode *func_node, SymbolTable *symbols)
{
    (void)symbols;
    
    const char *func_name = func_node->fun_stmt.name;
    
    // create symbol for function
    MasmSymbol *sym = masm_symbol_create(func_name, MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, sym);
    
    // create text section if not exists
    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);
    
    // add label
    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(func_name)));
    
    // lower body
    if (func_node->fun_stmt.body)
    {
        lower_stmt(masm, text, func_node->fun_stmt.body);
    }
    else
    {
        // just emit a ret for empty/external functions for now
        masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    }
}

Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols)
{
    MasmTarget target = masm_target_native();
    Masm *masm = masm_create(target);
    
    // iterate over top-level declarations
    if (ast->kind == AST_PROGRAM)
    {
        AstList *stmts = ast->program.stmts;
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *decl = stmts->items[i];
            if (decl->kind == AST_STMT_FUN)
            {
                lower_function(masm, decl, symbols);
            }
            // TODO: handle globals, etc.
        }
    }
    
    // Inject _start entry point
    // _start:
    //   call main
    //   mov rdi, rax
    //   mov rax, 60
    //   syscall
    
    // create _start symbol
    MasmSymbol *start_sym = masm_symbol_create("_start", MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, start_sym);
    
    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);
    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label("_start")));
    masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, masm_operand_label("main")));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, masm_operand_register(MASM_X86_RDI, 8), masm_operand_register(MASM_X86_RAX, 8)));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, masm_operand_register(MASM_X86_RAX, 8), masm_operand_imm(60)));
    masm_section_append_inst(text, masm_inst_0(MASM_OP_SYSCALL));
    
    return masm;
}
