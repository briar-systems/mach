class Node:
    def __init__(self, name, parent=None, children=None):
        self.name = name
        self.parent = parent
        self.children = children or []

class Expr(Node):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)


class Stmt(Node):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)


class Op(Expr):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)

class BinOp(Op):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)

class UnOp(Op):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)

class Assign(Stmt):
    def __init__(self, name, parent=None, children=None):
        super().__init__(name, parent, children)

class UseStmt(Stmt):
    def __init__(self, tok, name):
        super().__init__(tok)
        self.name = name

    def __repr__(self):
        return "UseStmt(%r, %r)" % (self.tok, self.name)

    def __str__(self):
        return "UseStmt(%s, %s)" % (self.tok, self.name)

    def __eq__(self, other):
        return self.tok == other.tok and self.name == other.name
 