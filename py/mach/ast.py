from .lex import *
from .node import *
from .tok import *


class InvalidTokenException(Exception):
    pass


class UnexpectedEOFException(Exception):
    pass


class Parser:
    def __init__(self, lex: Lex):
        self.lex = lex
        self.tok = None
        self.peek = None
        self.root = None

        # omit EOL tokens from lexer
        self.lex.omit(Kind.EOL)
        self.lex.omit(Kind.COMMENT)

        # populate tok and peek
        self.next()
        self.next()

    def next(self):
        self.prev = self.tok
        self.tok = self.peek
        self.peek = self.lex.next_tok()

    # exclusively handles blocks explicitly defined by braces
    def block(self):
        # skip opening brace
        if self.tok.type == Kind.LBRACE:
            self.next()

        # create a block
        block = Block(self.tok, [], self.root.syms)

        # process stmt and expr recursively
        while self.tok.type != Kind.RBRACE:
            block.stmts.append(self.stmt())

            if self.tok.type == Kind.EOF:
                raise UnexpectedEOFException(self.tok, "block remained unclosed at EOF")

        # skip closing brace
        if self.tok.type == Kind.RBRACE:
            self.next()

        return block

    def stmt_use(self): self.next(); return Node(self.tok)
    def stmt_fun(self): self.next(); return Node(self.tok)
    def stmt_var(self): self.next(); return Node(self.tok)
    def stmt_val(self): self.next(); return Node(self.tok)
    def stmt_type(self): self.next(); return Node(self.tok)
    def stmt_if(self): self.next(); return Node(self.tok)
    def stmt_else(self): self.next(); return Node(self.tok)
    def stmt_elif(self): self.next(); return Node(self.tok)
    def stmt_for(self): self.next(); return Node(self.tok)
    def stmt_in(self): self.next(); return Node(self.tok)
    def stmt_do(self): self.next(); return Node(self.tok)
    def stmt_match(self): self.next(); return Node(self.tok)
    def stmt_break(self): self.next(); return Node(self.tok)
    def stmt_cont(self): self.next(); return Node(self.tok)
    def stmt_ret(self): self.next(); return Node(self.tok)
    def stmt_true(self): self.next(); return Node(self.tok)
    def stmt_false(self): self.next(); return Node(self.tok)
    def stmt_ident(self): self.next(); return Node(self.tok)

    def stmt(self):
        if self.tok.type == Kind.IDENT:
            return self.stmt_ident()
        elif self.tok.type == Kind.LBRACE:
            return self.block()
        elif self.tok.type == Kind.USE:
            return self.stmt_use()
        elif self.tok.type == Kind.AS:
            return self.stmt_as()
        elif self.tok.type == Kind.FUN:
            return self.stmt_fun()
        elif self.tok.type == Kind.VAR:
            return self.stmt_var()
        elif self.tok.type == Kind.VAL:
            return self.stmt_val()
        elif self.tok.type == Kind.TYPE:
            return self.stmt_type()
        elif self.tok.type == Kind.IF:
            return self.stmt_if()
        elif self.tok.type == Kind.ELSE:
            return self.stmt_else()
        elif self.tok.type == Kind.ELIF:
            return self.stmt_elif()
        elif self.tok.type == Kind.FOR:
            return self.stmt_for()
        elif self.tok.type == Kind.IN:
            return self.stmt_in()
        elif self.tok.type == Kind.DO:
            return self.stmt_do()
        elif self.tok.type == Kind.MATCH:
            return self.stmt_match()
        elif self.tok.type == Kind.BREAK:
            return self.stmt_break()
        elif self.tok.type == Kind.CONT:
            return self.stmt_cont()
        elif self.tok.type == Kind.RET:
            return self.stmt_ret()
        elif self.tok.type == Kind.TRUE:
            return self.stmt_true()
        elif self.tok.type == Kind.FALSE:
            return self.stmt_false()
        else:
            raise InvalidTokenException(self.tok, "invalid token in stmt")

    # similar to block, but does not expect braces. Looks for EOF instead. This
    # is intended to be used for the top level of a file.
    def parse(self):
        self.root = Block(None, [], None)

        while self.tok.type != Kind.EOF:
            s = self.stmt()
            self.root.stmts.append(s)

        return self.root
