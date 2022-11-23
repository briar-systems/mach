class Node:
    def __init__(self, data):
        self.data = data
        self.next = None

class Stmt(Node):
    def __init__(self, nodes):
        self.nodes = nodes

    def __repr__(self):
        return "Stmt(%r)" % self.nodes

    def __str__(self):
        return "Stmt(%s)" % self.nodes

    def __eq__(self, other):
        return self.nodes == other.nodes

class Expr(Node):
    def __init__(self, nodes):
        self.nodes = nodes

    def __repr__(self):
        return "Expr(%r)" % self.nodes

    def __str__(self):
        return "Expr(%s)" % self.nodes

    def __eq__(self, other):
        return self.nodes == other.nodes
