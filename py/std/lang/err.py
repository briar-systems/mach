error_collection = []


class CompileError(Exception):
    def __init__(self, msg, file, pos):
        self.msg = msg
        self.file = file
        self.pos = pos

    def __str__(self):
        return f'{self.file}:{self.pos}: {self.msg}'


def add(msg, file, pos):
    error_collection.append(CompileError(msg, file, pos))


def print_errors():
    for err in error_collection:
        print(err)
