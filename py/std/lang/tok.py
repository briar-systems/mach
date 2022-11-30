from enum import Enum, auto

KINDS = {}


class Kind:
    def __init__(self, name, repr):
        self.name = name  # human readable name
        self.repr = repr  # in-code representation

        KINDS[name] = self

    def __str__(self):
        return self.name


class Token:
    def __init__(self, kind: Kind, pos: int, val):
        self.pos = pos
        self.val = val
        self.kind = kind

    def end(self):
        return self.pos + len(self)

    def __str__(self):
        return f'{self.kind}\t`{self.val}`'

    def __len__(self):
        return len(self.val)


# TODO:
# - `do`
# - `in`
INV = Kind("invalid", None)
UNK = Kind("unknown", None)
EOF = Kind("end of file", '\0')
EOL = Kind("end of line", '\n')
COMMENT = Kind("comment", "#")
IDENT = Kind("identifier", None)

LIT_INT = Kind("integer literal", None)
LIT_FLOAT = Kind("float literal", None)
LIT_STR = Kind("string literal", None)
LIT_CHAR = Kind("character literal", None)
LIT_BOOL = Kind("boolean literal", None)

UNOP_NOT = Kind("not", "!")
UNOP_POS = Kind("positive", "+")
UNOP_NEG = Kind("negative", "-")
UNOP_REF = Kind("reference", "?")
UNOP_DEREF = Kind("dereference", "@")

BINOP_ASSIGN = Kind("assignment", "=")
BINOP_EQ = Kind("equality", "==")
BINOP_NEQ = Kind("inequality", "!=")
BINOP_GTE = Kind("greater than or equal to", ">=")
BINOP_LTE = Kind("less than or equal to", "<=")
BINOP_GT = Kind("greater than", ">")
BINOP_LT = Kind("less than", "<")
BINOP_MUL = Kind("multiplication", "*")
BINOP_DIV = Kind("division", "/")
BINOP_EXP = Kind("exponentiation", "**")
BINOP_MOD = Kind("modulus", "%")
BINOP_AND = Kind("and", "&&")
BINOP_OR = Kind("or", "||")

BINOP_BIT_AND = Kind("bitwise and", "&")
BINOP_BIT_OR = Kind("bitwise or", "|")
BINOP_BIT_XOR = Kind("bitwise xor", "^")
BINOP_BIT_NOT = Kind("bitwise not", "~")
BINOP_SHL = Kind("shift left", "<<")
BINOP_SHR = Kind("shift right", ">>")

LPAREN = Kind("left parenthesis", "(")
RPAREN = Kind("right parenthesis", ")")
LBRACKET = Kind("left bracket", "[")
RBRACKET = Kind("right bracket", "]")
LBRACE = Kind("left brace", "{")
RBRACE = Kind("right brace", "}")
COMMA = Kind("comma", ",")
DOT = Kind("dot", ".")
COLON = Kind("colon", ":")
SEMICOLON = Kind("semicolon", ";")

KW_USE = Kind("use", "use")
KW_FUN = Kind("function", "fun")
KW_VAR = Kind("variable", "var")
KW_VAL = Kind("value", "val")
KW_TYPE = Kind("type", "type")
KW_IF = Kind("if", "if")
KW_ELSE = Kind("else", "else")
KW_ELIF = Kind("elif", "elif")
KW_FOR = Kind("for", "for")
KW_MATCH = Kind("match", "match")
KW_BREAK = Kind("break", "break")
KW_CONT = Kind("continue", "continue")
KW_RET = Kind("return", "return")


def find(val) -> Kind:
    for _, v in KINDS.items():
        if v.repr == val:
            return v

    return None


def get_row_col(src, pos):
    row = 1
    col = 1

    for i in range(pos):
        if src[i] == '\n':
            row += 1
            col = 1
        else:
            col += 1

    return row, col
