from .type import Type


class Int32(Type):
    def __init__(self, pos):
        super().__init__(pos, "i32", [])


class Int64(Type):
    def __init__(self, pos):
        super().__init__(pos, "i64", [])


class Float32(Type):
    def __init__(self, pos):
        super().__init__(pos, "f32", [])


class Float64(Type):
    def __init__(self, pos):
        super().__init__(pos, "f64", [])


class Bool(Type):
    def __init__(self, pos):
        super().__init__(pos, "bool", [])


class String(Type):
    def __init__(self, pos):
        super().__init__(pos, "str", [])


class Null(Type):
    def __init__(self, pos):
        super().__init__(pos, "null", [])


class Array(Type):
    def __init__(self, pos, type, size):
        super().__init__(pos, "array", [])
        self.type = type
        self.size = size
