from .node import *
from .type import *


class Expr(Node):
    def __init__(self, pos):
        super().__init__(pos)


class Ident(Expr):
    def __init__(self, pos, name, type, scope):
        super().__init__(pos)
        self.ident = name
        self.type = type
        self.scope = scope


class Ref(Expr):
    def __init__(self, pos, ident):
        super().__init__(pos)
        self.ident = ident


class Deref(Expr):
    def __init__(self, pos, ident):
        super().__init__(pos)
        self.ident = ident
