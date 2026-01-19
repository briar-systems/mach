#include "compiler/masm/isa/x86_64/asm.h"
#include "compiler/masm/isa/x86_64/x86_64.h"
#include "compiler/masm/operand.h"
#include "compiler/masm/section.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// parse register name to operand
static MasmOperand parse_reg(const char *name, uint8_t ptr_size)
{
    // 64-bit registers
    if (strcmp(name, "rax") == 0) return masm_operand_register(MASM_X86_RAX, 8);
    if (strcmp(name, "rbx") == 0) return masm_operand_register(MASM_X86_RBX, 8);
    if (strcmp(name, "rcx") == 0) return masm_operand_register(MASM_X86_RCX, 8);
    if (strcmp(name, "rdx") == 0) return masm_operand_register(MASM_X86_RDX, 8);
    if (strcmp(name, "rsi") == 0) return masm_operand_register(MASM_X86_RSI, 8);
    if (strcmp(name, "rdi") == 0) return masm_operand_register(MASM_X86_RDI, 8);
    if (strcmp(name, "rbp") == 0) return masm_operand_register(MASM_X86_RBP, 8);
    if (strcmp(name, "rsp") == 0) return masm_operand_register(MASM_X86_RSP, 8);
    if (strcmp(name, "r8") == 0) return masm_operand_register(MASM_X86_R8, 8);
    if (strcmp(name, "r9") == 0) return masm_operand_register(MASM_X86_R9, 8);
    if (strcmp(name, "r10") == 0) return masm_operand_register(MASM_X86_R10, 8);
    if (strcmp(name, "r11") == 0) return masm_operand_register(MASM_X86_R11, 8);
    if (strcmp(name, "r12") == 0) return masm_operand_register(MASM_X86_R12, 8);
    if (strcmp(name, "r13") == 0) return masm_operand_register(MASM_X86_R13, 8);
    if (strcmp(name, "r14") == 0) return masm_operand_register(MASM_X86_R14, 8);
    if (strcmp(name, "r15") == 0) return masm_operand_register(MASM_X86_R15, 8);

    // 32-bit registers
    if (strcmp(name, "eax") == 0) return masm_operand_register(MASM_X86_RAX, 4);
    if (strcmp(name, "ebx") == 0) return masm_operand_register(MASM_X86_RBX, 4);
    if (strcmp(name, "ecx") == 0) return masm_operand_register(MASM_X86_RCX, 4);
    if (strcmp(name, "edx") == 0) return masm_operand_register(MASM_X86_RDX, 4);
    if (strcmp(name, "esi") == 0) return masm_operand_register(MASM_X86_RSI, 4);
    if (strcmp(name, "edi") == 0) return masm_operand_register(MASM_X86_RDI, 4);
    if (strcmp(name, "ebp") == 0) return masm_operand_register(MASM_X86_RBP, 4);
    if (strcmp(name, "esp") == 0) return masm_operand_register(MASM_X86_RSP, 4);
    if (strcmp(name, "r8d") == 0) return masm_operand_register(MASM_X86_R8, 4);
    if (strcmp(name, "r9d") == 0) return masm_operand_register(MASM_X86_R9, 4);
    if (strcmp(name, "r10d") == 0) return masm_operand_register(MASM_X86_R10, 4);
    if (strcmp(name, "r11d") == 0) return masm_operand_register(MASM_X86_R11, 4);
    if (strcmp(name, "r12d") == 0) return masm_operand_register(MASM_X86_R12, 4);
    if (strcmp(name, "r13d") == 0) return masm_operand_register(MASM_X86_R13, 4);
    if (strcmp(name, "r14d") == 0) return masm_operand_register(MASM_X86_R14, 4);
    if (strcmp(name, "r15d") == 0) return masm_operand_register(MASM_X86_R15, 4);

    // 16-bit registers
    if (strcmp(name, "ax") == 0) return masm_operand_register(MASM_X86_RAX, 2);
    if (strcmp(name, "bx") == 0) return masm_operand_register(MASM_X86_RBX, 2);
    if (strcmp(name, "cx") == 0) return masm_operand_register(MASM_X86_RCX, 2);
    if (strcmp(name, "dx") == 0) return masm_operand_register(MASM_X86_RDX, 2);
    if (strcmp(name, "si") == 0) return masm_operand_register(MASM_X86_RSI, 2);
    if (strcmp(name, "di") == 0) return masm_operand_register(MASM_X86_RDI, 2);
    if (strcmp(name, "bp") == 0) return masm_operand_register(MASM_X86_RBP, 2);
    if (strcmp(name, "sp") == 0) return masm_operand_register(MASM_X86_RSP, 2);

    // 8-bit registers (low)
    if (strcmp(name, "al") == 0) return masm_operand_register(MASM_X86_RAX, 1);
    if (strcmp(name, "bl") == 0) return masm_operand_register(MASM_X86_RBX, 1);
    if (strcmp(name, "cl") == 0) return masm_operand_register(MASM_X86_RCX, 1);
    if (strcmp(name, "dl") == 0) return masm_operand_register(MASM_X86_RDX, 1);
    if (strcmp(name, "sil") == 0) return masm_operand_register(MASM_X86_RSI, 1);
    if (strcmp(name, "dil") == 0) return masm_operand_register(MASM_X86_RDI, 1);
    if (strcmp(name, "bpl") == 0) return masm_operand_register(MASM_X86_RBP, 1);
    if (strcmp(name, "spl") == 0) return masm_operand_register(MASM_X86_RSP, 1);
    if (strcmp(name, "r8b") == 0) return masm_operand_register(MASM_X86_R8, 1);
    if (strcmp(name, "r9b") == 0) return masm_operand_register(MASM_X86_R9, 1);
    if (strcmp(name, "r10b") == 0) return masm_operand_register(MASM_X86_R10, 1);
    if (strcmp(name, "r11b") == 0) return masm_operand_register(MASM_X86_R11, 1);
    if (strcmp(name, "r12b") == 0) return masm_operand_register(MASM_X86_R12, 1);
    if (strcmp(name, "r13b") == 0) return masm_operand_register(MASM_X86_R13, 1);
    if (strcmp(name, "r14b") == 0) return masm_operand_register(MASM_X86_R14, 1);
    if (strcmp(name, "r15b") == 0) return masm_operand_register(MASM_X86_R15, 1);

    (void)ptr_size;
    return masm_operand_none();
}

// parse operand: register, immediate, or memory
static MasmOperand parse_operand(const char *str, uint8_t ptr_size)
{
    // try register first
    MasmOperand reg = parse_reg(str, ptr_size);
    if (reg.kind != MASM_OPERAND_NONE)
    {
        return reg;
    }

    // parse simple memory operands: [reg] or [reg+imm] or [reg-imm]
    if (str[0] == '[')
    {
        size_t len = strlen(str);
        if (len >= 3 && str[len - 1] == ']')
        {
            char   inner[64];
            size_t copy_len = len - 2 < sizeof(inner) - 1 ? len - 2 : sizeof(inner) - 1;
            memcpy(inner, str + 1, copy_len);
            inner[copy_len] = '\0';

            // split on '+' or '-' if present
            char *plus  = strchr(inner, '+');
            char *minus = strchr(inner, '-');
            char *sep   = plus ? plus : minus;
            bool  neg   = (sep == minus);

            char *reg_str = inner;
            char *off_str = NULL;
            if (sep)
            {
                *sep    = '\0';
                off_str = sep + 1;
            }

            MasmOperand base = parse_reg(reg_str, ptr_size);
            if (base.kind == MASM_OPERAND_REGISTER)
            {
                int64_t disp = 0;
                if (off_str)
                {
                    disp = strtoll(off_str, NULL, 0);
                    if (neg) disp = -disp;
                }
                return masm_operand_memory_simple(base.reg.id, (int32_t)disp, ptr_size);
            }
        }
    }

    // parse immediate (number)
    char *end;
    long  val = strtol(str, &end, 0);
    if (str != end && *end == '\0')
    {
        return masm_operand_imm(val);
    }

    // unrecognized
    return masm_operand_none();
}

// trim leading whitespace in place (returns pointer into same buffer)
static char *trim_leading(char *s)
{
    while (*s == ' ' || *s == '\t')
    {
        s++;
    }
    return s;
}

// trim trailing whitespace in place
static void trim_trailing(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r'))
    {
        s[--len] = '\0';
    }
}

// parse two-operand instruction: "op dst, src"
static bool parse_two_op(char *args, MasmOperand *dst, MasmOperand *src, uint8_t ptr_size)
{
    char *comma = strchr(args, ',');
    if (!comma) return false;

    *comma      = '\0';
    char *dst_s = trim_leading(args);
    char *src_s = trim_leading(comma + 1);
    trim_trailing(dst_s);
    trim_trailing(src_s);

    *dst = parse_operand(dst_s, ptr_size);
    *src = parse_operand(src_s, ptr_size);

    return dst->kind != MASM_OPERAND_NONE && src->kind != MASM_OPERAND_NONE;
}

// parse one-operand instruction: "op operand"
static bool parse_one_op(char *args, MasmOperand *op, uint8_t ptr_size)
{
    char *s = trim_leading(args);
    trim_trailing(s);
    *op = parse_operand(s, ptr_size);
    return op->kind != MASM_OPERAND_NONE;
}

void masm_x86_parse_inline_asm(MasmSection *section, const char *content, uint8_t ptr_size)
{
    char *line    = strdup(content);
    char *saveptr = NULL;
    char *token   = strtok_r(line, "\n;", &saveptr);

    while (token)
    {
        token = trim_leading(token);

        // strip inline comments starting with '#'
        char *comment = strchr(token, '#');
        if (comment)
        {
            *comment = '\0';
        }

        trim_trailing(token);

        if (*token == '\0')
        {
            token = strtok_r(NULL, "\n;", &saveptr);
            continue;
        }

        MasmOperand dst, src;

        // syscall (no operands)
        if (strcmp(token, "syscall") == 0)
        {
            masm_section_append_inst(section, masm_x86_inst_0(MASM_OP_X86_SYSCALL));
        }
        // ret (no operands)
        else if (strcmp(token, "ret") == 0)
        {
            masm_section_append_inst(section, masm_x86_inst_0(MASM_OP_X86_RET));
        }
        // call label
        else if (strncmp(token, "call ", 5) == 0)
        {
            char *label = trim_leading(token + 5);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_CALL_REL, masm_operand_label(label)));
        }
        // jmp label
        else if (strncmp(token, "jmp ", 4) == 0)
        {
            char *label = trim_leading(token + 4);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JMP_REL, masm_operand_label(label)));
        }
        // je/jz label
        else if (strncmp(token, "je ", 3) == 0 || strncmp(token, "jz ", 3) == 0)
        {
            char *label = trim_leading(token + 3);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JE, masm_operand_label(label)));
        }
        // jne/jnz label
        else if (strncmp(token, "jne ", 4) == 0 || strncmp(token, "jnz ", 4) == 0)
        {
            char *label = trim_leading(token + 4);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JNE, masm_operand_label(label)));
        }
        // jl label
        else if (strncmp(token, "jl ", 3) == 0)
        {
            char *label = trim_leading(token + 3);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JL, masm_operand_label(label)));
        }
        // jge label
        else if (strncmp(token, "jge ", 4) == 0)
        {
            char *label = trim_leading(token + 4);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JGE, masm_operand_label(label)));
        }
        // jg label
        else if (strncmp(token, "jg ", 3) == 0)
        {
            char *label = trim_leading(token + 3);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JG, masm_operand_label(label)));
        }
        // jle label
        else if (strncmp(token, "jle ", 4) == 0)
        {
            char *label = trim_leading(token + 4);
            trim_trailing(label);
            masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_JLE, masm_operand_label(label)));
        }
        // cmp dst, src
        else if (strncmp(token, "cmp ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_CMP_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_CMP_RR, dst, src));
                }
            }
        }
        // test dst, src
        else if (strncmp(token, "test ", 5) == 0)
        {
            if (parse_two_op(token + 5, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_TEST_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_TEST_RR, dst, src));
                }
            }
        }
        // mov dst, src
        else if (strncmp(token, "mov ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (dst.kind == MASM_OPERAND_MEMORY && src.kind == MASM_OPERAND_REGISTER)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOV_MR, dst, src));
                }
                else if (dst.kind == MASM_OPERAND_REGISTER && src.kind == MASM_OPERAND_MEMORY)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOV_RM, dst, src));
                }
                else if (dst.kind == MASM_OPERAND_REGISTER && src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOV_RI, dst, src));
                }
                else if (dst.kind == MASM_OPERAND_REGISTER && src.kind == MASM_OPERAND_REGISTER)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOV_RR, dst, src));
                }
            }
        }
        // movzx dst, src (zero extend)
        else if (strncmp(token, "movzx ", 6) == 0)
        {
            if (parse_two_op(token + 6, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_MEMORY)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOVZX_RM, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOVZX_RR, dst, src));
                }
            }
        }
        // movsx dst, src (sign extend)
        else if (strncmp(token, "movsx ", 6) == 0)
        {
            if (parse_two_op(token + 6, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_MEMORY)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOVSX_RM, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_MOVSX_RR, dst, src));
                }
            }
        }
        // lea dst, src
        else if (strncmp(token, "lea ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_LEA, dst, src));
            }
        }
        // add dst, src
        else if (strncmp(token, "add ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_ADD_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_ADD_RR, dst, src));
                }
            }
        }
        // sub dst, src
        else if (strncmp(token, "sub ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SUB_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SUB_RR, dst, src));
                }
            }
        }
        // and dst, src
        else if (strncmp(token, "and ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_AND_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_AND_RR, dst, src));
                }
            }
        }
        // or dst, src
        else if (strncmp(token, "or ", 3) == 0)
        {
            if (parse_two_op(token + 3, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_OR_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_OR_RR, dst, src));
                }
            }
        }
        // xor dst, src
        else if (strncmp(token, "xor ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_XOR_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_XOR_RR, dst, src));
                }
            }
        }
        // shl dst, src
        else if (strncmp(token, "shl ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SHL_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SHL_RC, dst, src));
                }
            }
        }
        // shr dst, src
        else if (strncmp(token, "shr ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SHR_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SHR_RC, dst, src));
                }
            }
        }
        // sar dst, src
        else if (strncmp(token, "sar ", 4) == 0)
        {
            if (parse_two_op(token + 4, &dst, &src, ptr_size))
            {
                if (src.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SAR_RI, dst, src));
                }
                else
                {
                    masm_section_append_inst(section, masm_x86_inst_2(MASM_OP_X86_SAR_RC, dst, src));
                }
            }
        }
        // push src
        else if (strncmp(token, "push ", 5) == 0)
        {
            if (parse_one_op(token + 5, &src, ptr_size))
            {
                masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_PUSH_R, src));
            }
        }
        // pop dst
        else if (strncmp(token, "pop ", 4) == 0)
        {
            if (parse_one_op(token + 4, &dst, ptr_size))
            {
                masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_POP_R, dst));
            }
        }
        // neg dst
        else if (strncmp(token, "neg ", 4) == 0)
        {
            if (parse_one_op(token + 4, &dst, ptr_size))
            {
                masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_NEG_R, dst));
            }
        }
        // not dst
        else if (strncmp(token, "not ", 4) == 0)
        {
            if (parse_one_op(token + 4, &dst, ptr_size))
            {
                masm_section_append_inst(section, masm_x86_inst_1(MASM_OP_X86_NOT_R, dst));
            }
        }
        // unknown instruction - silently ignore for now
        // (could emit a warning in the future)

        token = strtok_r(NULL, "\n;", &saveptr);
    }

    free(line);
}
