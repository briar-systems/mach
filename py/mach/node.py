from .tok import *


class Node:
    def __init__(self, tok):
        self.tok = tok


class Block(Node):
    def __init__(self, tok, stmts, sym):
        super().__init__(tok)
        self.stmts = stmts
        self.syms = {}  # symbol table (inherited from parent)

    def get_sym(self, name):
        if name in self.syms:
            return self.syms[name]
        return None


class Expr(Node):
    pass


class Stmt(Node):
    pass
