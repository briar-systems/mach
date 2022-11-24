from .lex import *


class InvalidTokenException(Exception):
    pass


class Parser:
    def __init__(self, lex: Lex):
        self.lex = lex
        self.tok = None
        self.peek = None
        self.next()
        self.next()

    def next(self):
        self.tok = self.peek
        self.peek = self.lex.next_tok()

    def check(self, tok_type: TokenType):
        return self.tok.type == tok_type

    def check_peek(self, tok_type: TokenType):
        return self.lex.peek_tok().type == tok_type

    def match(self, tok_type: TokenType):
        if self.check(tok_type):
            self.next()
            return True

        self.next()
        return False

    def nl(self):
        if not self.match(TokenType.EOL):
            return False
        while self.check(TokenType.EOL):
            self.next()

    def expr(self):
        pass

    def stmt(self):
        if self.check(TokenType.EOF):
            return None
        if self.check(TokenType.EOL):
            return None
        if self.check(TokenType.IDENT):
            return self.expr()
        if self.check(TokenType.INT):
            return self.expr()
        if self.check(TokenType.FLOAT):
            return self.expr()

        self.nl()

    def exec(self):
        while not self.check(TokenType.EOF):
            self.stmt()
            self.match(TokenType.EOL)
