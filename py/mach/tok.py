from enum import Enum, auto


class Token:
    def __init__(self, text, type):
        self.text = text
        self.type = type


class TokenType(Enum):
    # representational tokens
    UNK = -1
    EOF = auto()
    EOL = auto()

    # # builtins
    # IDENT = 0x0010
    # USE = 0x0010
    # PUB = 0x0011
    # FUN = 0x0012
    # VAR = 0x0013
    # VAL = 0x0014
    # REF = 0x0015

    # # control flow
    # IF = 0x0020
    # ELIF = 0x0021
    # ELSE = 0x0022
    # FOR = 0x0023
    # WHILE = 0x0024
    # MATCH = 0x0025
    # CASE = 0x0026
    # BREAK = 0x0027
    # CONTINUE = 0x0028
    # RETURN = 0x0029

    # # constant types
    # INT = 0x0030
    # FLOAT = 0x0031
    # BOOL = 0x0032
    # STRING = 0x0033
    # CHAR = 0x0034
    # NIL = 0x0035

    # literal tokens
    COMMENT = auto()
    IDENT = auto()
    INT = auto()
    FLOAT = auto()
    BOOL = auto()
    STRING = auto()
    CHAR = auto()
    NIL = auto()

    # unary operators
    NOT = auto()
    POS = auto()
    NEG = auto()
    PTR = auto()
    DEREF = auto()

    # binary operators
    ASSIGN = auto()
    EQ = auto()
    NEQ = auto()
    GTE = auto()
    LTE = auto()
    GT = auto()
    LT = auto()
    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    EXP = auto()
    MOD = auto()
    AND = auto()
    OR = auto()
    BIT_AND = auto()
    BIT_OR = auto()
    BIT_XOR = auto()
    BIT_NOT = auto()
    SHL = auto()
    SHR = auto()

    # other
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    COMMA = auto()
    COLON = auto()
    SEMICOLON = auto()
    DOT = auto()
    FSLASH = auto()
    BSLASH = auto()
    ASTERISK = auto()
