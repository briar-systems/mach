from . import tok
from . import err

from .ast import node
from .ast import expr
from .ast import stmt
from .ast import scope
from .ast import type


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.scope = scope.Scope()
        self.pos = 0

    def tok(self) -> tok.Token:
        return self.tokens[self.pos]

    def next(self):
        if self.pos < len(self.tokens):
            self.pos += 1

    def peek(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos+1]

        return tok.Token(tok.EOF, self.pos, "")

    def check(self, val, str):
        if val.kind != str:
            return False
        if val.kind == tok.EOF:
            return False

        return True

    def skip_eol(self):
        while self.check(self.tok(), tok.EOL):
            self.next()

    def get_from(self, beg, end=None):
        return self.tokens[beg:end or self.pos + 1]

    def expr(self) -> expr.Expr:
        pass

    def stmt(self) -> stmt.Stmt:
        pass

    def stmt_use(self) -> stmt.Stmt:
        if not self.check(self.peek(), tok.LIT_STR):
            err.add("expected string literal", "parse", self.peek().pos)
            return None

        self.next()  # skip use kw

        path = self.tok().val

        return stmt.Use(self.pos, path)

    def stmt_var(self) -> stmt.Stmt:
        if not self.check(self.peek(), tok.IDENT):
            err.add("expected identifier", "parse", self.peek().pos)
            return None

        self.next()  # skip var kw

        ident = self.tok().val
        self.next()

        if not self.check(self.tok(), tok.ASSIGN):
            err.add("expected assignment", "parse", self.tok().pos)
            return None

        self.next()  # skip assign

        expr = self.expr()
        if expr is None:
            return None

        return stmt.Var(self.pos, ident, expr)

    def stmt_val(self) -> stmt.Stmt:
        if not self.check(self.peek(), tok.IDENT):
            err.add("expected identifier", "parse", self.peek().pos)
            return None

        self.next()  # skip val kw

        ident = self.tok().val
        self.next()

        if not self.check(self.tok(), tok.ASSIGN):
            err.add("expected assignment", "parse", self.tok().pos)
            return None

        self.next()  # skip assign
        expr = self.expr()
        if expr is None:
            return None

        return stmt.Val(self.pos, ident, expr)

    def stmt_type(self) -> stmt.Stmt:
        if not self.check(self.peek(), tok.IDENT):
            err.add("expected identifier", "parse", self.peek().pos)
            return None

        self.next()  # skip type kw

        name = self.tok().val
        self.next()

        if not self.check(self.tok(), tok.LBRACE):
            err.add("expected '{'", "parse", self.tok().pos)
            return None

        self.next()  # skip lbrace
        self.skip_eol()

        fields = []
        while not self.check(self.tok(), tok.RBRACE):
            if not self.check(self.tok(), tok.IDENT):
                err.add("expected identifier", "parse", self.tok().pos)
                return None

            var_ident = self.tok().val
            self.next()  # skip var_ident

            if not self.check(self.tok(), tok.COLON):
                err.add("expected ':'", "parse", self.tok().pos)
                return None

            self.next()  # skip colon

            # TODO: parse type fields
            field = None
            match self.tok():
                case tok.IDENT: pass
                case tok.REF:
                    self.next()  # skip ref token
                    pass
                case _:
                    err.add("expected type", "parse", self.tok().pos)
                    return None

            fields.append(type.Field(self.tok().pos, var_ident, field))

            self.next()  # skip type

            if not self.check(self.tok(), tok.COMMA):
                self.skip_eol()
                if not self.check(self.tok(), tok.RBRACE):
                    err.add("expected '}'", "parse", self.tok().pos)
                    return None

            self.next()  # skip comma

            self.skip_eol()

        self.next()  # skip rbrace

        return stmt.Type(self.pos, name, fields)

    def stmt_fun(self) -> stmt.Stmt:
        if not self.check(self.peek(), tok.IDENT):
            err.add("expected identifier", "parse", self.peek().pos)
            return None

        self.next()  # skip fun kw

        type_name = None
        if self.check(self.peek(), tok.DOT):
            self.next()  # skip dot
            type_name = self.tok().val
            self.next()  # skip type ident

        if not self.check(self.tok(), tok.IDENT):
            err.add("expected identifier", "parse", self.tok().pos)
            return None

        name = self.tok().val
        self.next()  # skip name

        if not self.check(self.tok(), tok.LPAREN):
            err.add("expected '('", "parse", self.tok().pos)
            return None

        self.next()  # skip lparen

        params = []
        while not self.check(self.tok(), tok.RPAREN):
            if not self.check(self.tok(), tok.IDENT):
                err.add("expected identifier", "parse", self.tok().pos)
                return None

            ident = self.tok().val
            self.next()

            if not self.check(self.tok(), tok.COLON):
                err.add("expected ':'", "parse", self.tok().pos)
                return None

            self.next()  # skip colon

            param_type = None
            match self.tok():
                case tok.IDENT: pass
                case tok.REF:
                    self.next()  # skip ref token
                    pass
                case _:
                    err.add("expected type", "parse", self.tok().pos)
                    return None

            params.append(node.Param(self.tok().pos, ident, param_type))

            self.next()  # skip type

            if not self.check(self.tok(), tok.COMMA):
                if not self.check(self.tok(), tok.RPAREN):
                    err.add("expected ',' or ')'", "parse", self.tok().pos)
                    return None

            self.next()  # skip comma

        self.next()  # skip rparen

        # optional return type
        ret_type = None
        if self.check(self.tok(), tok.IDENT):
            ret_type = self.tok().val
            self.next()

        if not self.check(self.tok(), tok.LBRACE):
            err.add("expected '{'", "parse", self.tok().pos)
            return None

        self.next()  # skip lbrace
        self.skip_eol()

        body = []
        while not self.check(self.tok(), tok.RBRACE):
            body_stmt = self.stmt()
            if body_stmt is None:
                return None

            body.append(body_stmt)

            self.skip_eol()

        self.next()  # skip rbrace

        return stmt.Fun(self.pos, name, type_name, params, ret_type, body)

    def exec(self):
        block = node.Block(self.pos)

        while True:
            self.skip_eol()

            stmt = None
            match self.tok():
                case tok.KW_USE:
                    stmt = self.stmt_use()
                case tok.KW_FUN:
                    stmt = self.stmt_fun()
                case tok.KW_VAR:
                    stmt = self.stmt_var()
                case tok.KW_VAL:
                    stmt = self.stmt_val()
                case tok.KW_TYPE:
                    stmt = self.stmt_type()

            self.next()
            if stmt is not None:
                block.add(stmt)
                continue

            break

        # expect EOF
        if not self.check(self.tok(), tok.EOF):
            err.add("expected EOF", 'parse', self.tok().pos)
