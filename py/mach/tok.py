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

    # literal tokens
    COMMENT = auto()
    IDENT = auto()
    INT = auto()
    FLOAT = auto()
    STRING = auto()
    CHAR = auto()

    # unary operators
    NOT = auto()
    POS = auto()
    NEG = auto()
    REF = auto()
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
    LBRACKET = auto()
    RBRACKET = auto()
    LBRACE = auto()
    RBRACE = auto()
    COMMA = auto()
    DOT = auto()
    COLON = auto()
    SEMICOLON = auto()


tokens = {
    TokenType.UNK: '\\',
    TokenType.EOL: '\n',
    TokenType.EOF: '\0',
    TokenType.COMMENT: '#foo',
    TokenType.INT: '0',
    TokenType.FLOAT: '0.0',
    TokenType.STRING: '\"foo\"',
    TokenType.CHAR: '\'f\'',
    TokenType.NOT: '!',
    TokenType.POS: '+',
    TokenType.NEG: '-',
    TokenType.REF: '?',
    TokenType.DEREF: '@',
    TokenType.ASSIGN: '=',
    TokenType.EQ: '==',
    TokenType.NEQ: '!=',
    TokenType.GTE: '>=',
    TokenType.LTE: '<=',
    TokenType.GT: '>',
    TokenType.LT: '<',
    TokenType.MUL: '*',
    TokenType.DIV: '/',
    TokenType.EXP: '**',
    TokenType.MOD: '%',
    TokenType.AND: '&&',
    TokenType.OR: '||',
    TokenType.BIT_AND: '&',
    TokenType.BIT_OR: '|',
    TokenType.BIT_XOR: '^',
    TokenType.BIT_NOT: '~',
    TokenType.SHL: '<<',
    TokenType.SHR: '>>',
    TokenType.LPAREN: '(',
    TokenType.RPAREN: ')',
    TokenType.LBRACKET: '[',
    TokenType.RBRACKET: ']',
    TokenType.LBRACE: '{',
    TokenType.RBRACE: '}',
    TokenType.COMMA: ',',
    TokenType.DOT: '.',
    TokenType.COLON: ':',
    TokenType.SEMICOLON: ';',
}
