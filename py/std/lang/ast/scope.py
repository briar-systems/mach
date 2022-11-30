class Scope:
    def __init__(self, name, parent):
        self.name = name
        self.parent = parent
        self.idents = {}

    def add_ident(self, ident):
        self.idents[ident.name] = ident

    def get_ident(self, name):
        if name in self.idents:
            return self.idents[name]
        if self.parent:
            return self.parent.get_ident(name)
        return None


root = Scope(None)
