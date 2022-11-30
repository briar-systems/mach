class Node:
    def __init__(self, pos):
        self.pos = pos


class Block(Node):
    def __init__(self, pos):
        super().__init__(pos)
        self.stmts = []

    def add(self, stmt):
        self.stmts.append(stmt)


class Param(Node):
    def __init__(self, pos, name, type):
        super().__init__(pos)
        self.name = name
        self.type = type
