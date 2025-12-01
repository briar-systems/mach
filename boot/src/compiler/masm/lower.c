#include "compiler/masm/lower.h"
#include "compiler/masm/instruction.h"
#include "compiler/masm/isa/x86_64.h"
#include <stdio.h>
#include <string.h>

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
    else if (expr->kind == AST_EXPR_BINARY)
    {
        MasmOperand left = lower_expr(masm, text, expr->binary_expr.left);
        MasmOperand right = lower_expr(masm, text, expr->binary_expr.right);
        
        MasmOperand result = masm_operand_register(MASM_X86_RAX, 8);
        
        // move left operand to result register
        if (left.kind != MASM_OPERAND_REGISTER || left.reg.id != MASM_X86_RAX)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
        }
        
        // if right is immediate, need to load into register first for reg-reg operations
        MasmOperand right_op = right;
        if (right.kind == MASM_OPERAND_IMM)
        {
            MasmOperand temp = masm_operand_register(MASM_X86_RCX, 8);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, right));
            right_op = temp;
        }
        
        uint32_t opcode = 0;
        switch (expr->binary_expr.op)
        {
            case TOKEN_PLUS:  opcode = MASM_OP_ADD; break;
            case TOKEN_MINUS: opcode = MASM_OP_SUB; break;
            case TOKEN_STAR:  opcode = MASM_OP_IMUL; break;
            case TOKEN_SLASH: 
                opcode = MASM_OP_IDIV;
                break;
            default:
                return masm_operand_none();
        }
        
        if (opcode == MASM_OP_IDIV)
        {
            masm_section_append_inst(text, masm_inst_0(MASM_OP_CQO));
            masm_section_append_inst(text, masm_inst_1(opcode, right_op));
        }
        else
        {
            masm_section_append_inst(text, masm_inst_2(opcode, result, right_op));
        }
        
        return result;
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
    
    MasmSymbol *sym = masm_symbol_create(func_name, MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, sym);
    
    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);
    
    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(func_name)));
    
    if (func_node->fun_stmt.body)
    {
        lower_stmt(masm, text, func_node->fun_stmt.body);
    }
    else
    {
        masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    }
}

Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols)
{
    MasmTarget target = masm_target_native();
    Masm *masm = masm_create(target);
    
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
        }
    }
    
    // inject _start entry point
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
