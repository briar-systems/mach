from .node import *
from .expr import *


class Type(Node):
    def __init__(self, pos, name, fields):
        super().__init__(pos)
        self.name = name
        self.fields = fields


class Field(Node):
    def __init__(self, pos, name, type):
        super().__init__(pos)
        self.name = name
        self.type = type
