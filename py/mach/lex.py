from .tok import TokenType, Token


class Lex:
    def __init__(self, input):
        self.src = input + "\n"
        self.val = ""
        self.row = +1  # index at 1
        self.col = +0  # index at 1
        self.pos = -1  # index at 0

        self.next()

    def next(self):
        self.pos += 1
        self.col += 1
        if self.pos >= len(self.src):
            self.val = '\0'
            self.pos -= 1
            self.col -= 1
        else:
            self.val = self.src[self.pos]

    def peek(self):
        if self.pos + 1 >= len(self.src):
            return '\0'

        return self.src[self.pos + 1]

    def get_ident(self):
        start = self.pos
        # make sure the first character is a letter
        if not self.val.isalpha() and self.val != '_':
            return self.emit(TokenType.INV)

        while self.peek().isalnum():
            self.next()

        return self.emit(TokenType.IDENT, self.src[start:self.pos+1])

    def get_string(self):
        self.next()
        start = self.pos
        while self.peek() != '"':
            self.next()

        self.next()
        return self.emit(TokenType.STRING, self.src[start:self.pos])

    def get_char(self):
        self.next()
        start = self.pos
        while self.peek() != '\'':
            self.next()

        self.next()
        return self.emit(TokenType.CHAR, self.src[start:self.pos])

    def get_comment(self):
        self.next()
        start = self.pos
        while self.peek() != '\n':
            self.next()

        self.next()
        return self.emit(TokenType.COMMENT, self.src[start:self.pos])

    def get_number(self):
        start = self.pos

        # check for hex/oct/bin
        if self.val == '0':
            # hex
            if self.val in 'xX':
                self.next()
                self.next()
                while self.peek().isalnum():
                    self.next()

                return self.emit(TokenType.INT, int(self.src[start:self.pos+1], 16))

            # oct/bin
            if self.val in 'oObB':
                self.next()
                self.next()
                while self.peek().isnumeric():
                    self.next()

                return self.emit(TokenType.INT, int(self.src[start:self.pos+1], 8))

        # scan for digits
        while self.peek().isdigit() or self.val == '_':
            self.next()

        # check for float
        if self.peek() == '.':
            self.next()

            while self.peek().isdigit() or self.val == '_':
                self.next()

            return self.emit(TokenType.FLOAT, float(self.src[start:self.pos+1]))

        return self.emit(TokenType.INT, int(self.src[start:self.pos+1]))

    def skip_whitespace(self):
        while self.val == ' ' or self.val == '\t' or self.val == '\r':
            self.next()

    def emit(self, type, val=None):
        val = val if val is not None else self.val
        size = len(str(val)) - 1
        return Token(
            self.pos,
            self.row,
            self.col - size,
            val,
            type
        )

    def next_tok(self):
        self.skip_whitespace()
        tok = None

        # TODO: clamp the size on this a bit ya? Maybe `get_operator`?
        match self.val:
            case '\0':
                tok = self.emit(TokenType.EOF)
            case '\n':
                tok = self.emit(TokenType.EOL)
                self.row += 1
                self.col = 0
            case '#':
                tok = self.get_comment()
                self.row += 1
                self.col = 0
            case '"':
                tok = self.get_string()
            case '\'':
                tok = self.get_char()
            case '+':
                tok = self.emit(TokenType.POS)
            case '-':
                tok = self.emit(TokenType.NEG)
            case '%':
                tok = self.emit(TokenType.MOD)
            case '/':
                tok = self.emit(TokenType.DIV)
            case '(':
                tok = self.emit(TokenType.LPAREN)
            case ')':
                tok = self.emit(TokenType.RPAREN)
            case '[':
                tok = self.emit(TokenType.LBRACKET)
            case ']':
                tok = self.emit(TokenType.RBRACKET)
            case '{':
                tok = self.emit(TokenType.LBRACE)
            case '}':
                tok = self.emit(TokenType.RBRACE)
            case ',':
                tok = self.emit(TokenType.COMMA)
            case ';':
                tok = self.emit(TokenType.SEMICOLON)
            case ':':
                tok = self.emit(TokenType.COLON)
            case '.':
                tok = self.emit(TokenType.DOT)
            case '~':
                tok = self.emit(TokenType.BIT_NOT)
            case '^':
                tok = self.emit(TokenType.BIT_XOR)
            case '?':
                tok = self.emit(TokenType.REF)
            case '@':
                tok = self.emit(TokenType.DEREF)
            case '!':
                if self.peek() == '=':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.NEQ, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.NOT)
            case '*':
                if self.peek() == '*':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.EXP, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.MUL)
            case '&':
                if self.peek() == '&':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.AND, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.BIT_AND)
            case '=':
                if self.peek() == '=':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.EQ, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.ASSIGN)
            case '|':
                if self.peek() == '|':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.OR, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.BIT_OR)
            case '<':
                if self.peek() == '=':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.LTE, self.src[start:self.pos+1])
                elif self.peek() == '<':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.SHL, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.LT)
            case '>':
                if self.peek() == '=':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.GTE, self.src[start:self.pos+1])
                elif self.peek() == '>':
                    start = self.pos
                    self.next()
                    tok = self.emit(TokenType.SHR, self.src[start:self.pos+1])
                else:
                    tok = self.emit(TokenType.GT)
            case _:
                if self.val.isalpha():
                    tok = self.get_ident()
                elif self.val.isdigit():
                    tok = self.get_number()
                else:
                    tok = self.emit(TokenType.UNK)

        if tok is None:
            tok = self.emit(TokenType.INV, self.val)

        if tok.type != TokenType.EOL:
            print(f'{tok.row:03d}:{tok.col:03d} | ', end='')
            print(tok.type, f"\'{tok.val}\'",
                  sep=' ' * (20 - len(str(tok.type))))
        else:
            print("--------|--------")

        self.next()

        return tok
