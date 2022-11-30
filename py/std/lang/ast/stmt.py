from .node import *


class Stmt(Node):
    def __init__(self, pos):
        super().__init__(pos)


class Use(Stmt):
    def __init__(self, pos, path):
        super().__init__(pos)
        self.path = path


class Var(Stmt):
    def __init__(self, pos, ident, expr):
        super().__init__(pos)
        self.ident = ident
        self.expr = expr


class Fun(Stmt):
    def __init__(self, pos, ident, params, ret, body):
        super().__init__(pos)
        self.ident = ident
        self.params = params
        self.ret = ret
        self.body = body
