from .tok import TokenType, Token


class Lex:
    def __init__(self, input):
        self.src = input + "\n"
        self.char = ''
        self.pos = -1

        # line is only used for error reporting
        self.line = 1

        self.next()

    def next(self):
        self.pos += 1
        if self.pos >= len(self.src):
            self.char = '\0'
        else:
            self.char = self.src[self.pos]

    def peek(self):
        if self.pos + 1 >= len(self.src):
            return '\0'

        return self.src[self.pos + 1]

    def abort(self, msg):
        raise Exception("lex error: " + msg)

    def get_ident(self):
        start = self.pos
        while self.peek().isalnum():
            self.next()

        return Token(self.src[start:self.pos + 1], TokenType.IDENT)

    def get_number(self):
        start = self.pos
        while self.peek().isdigit() or self.peek() == '_':
            self.next()

        if self.peek() == '.':
            self.next()
            if not self.peek().isdigit():
                self.abort("unexpected character " + self.char)

            while self.peek().isdigit() or self.peek() == '_':
                self.next()

            return Token(self.src[start:self.pos + 1], TokenType.FLOAT)

        return Token(self.src[start:self.pos + 1], TokenType.INT)

    def get_string(self):
        start = self.pos
        self.next()
        while self.char != '"':
            self.next()
        return Token(self.src[start + 1:self.pos], TokenType.STRING)

    def get_char(self):
        start = self.pos
        self.next()
        while self.char != '\'':
            self.next()
        return Token(self.src[start + 1:self.pos], TokenType.CHAR)

    def get_comment(self):
        start = self.pos
        while self.peek() != '\n':
            self.next()
        return Token(self.src[start+1:self.pos + 1], TokenType.COMMENT)

    def skip_whitespace(self):
        while self.char == ' ' or self.char == '\t' or self.char == '\r':
            self.next()

    def next_tok(self):
        # print('\'', self.char if self.char != '\n' else ';', '\'', sep='')

        self.skip_whitespace()
        tok = Token(None, None)

        match self.char:
            case '\0':
                tok = Token(self.char, TokenType.EOF)
                self.next()
            case '\n':
                self.line += 1
                tok = Token(';', TokenType.EOL)
                self.next()
            case ';':
                tok = Token(self.char, TokenType.EOL)
                self.next()
            case '#':
                tok = self.get_comment()
                self.next()
            case '"':
                tok = self.get_string()
                self.next()
            case '\'':
                tok = self.get_char()
                self.next()
            case '+':
                tok = Token(self.char, TokenType.POS)
                self.next()
            case '-':
                tok = Token(self.char, TokenType.NEG)
                self.next()
            case '%':
                tok = Token(self.char, TokenType.MOD)
                self.next()
            case '/':
                tok = Token(self.char, TokenType.DIV)
                self.next()
            case '!':
                if self.peek() == '=':
                    tok = Token(self.char, TokenType.NEQ)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.NOT)
                    self.next()
            case '*':
                if self.peek() == '*':
                    tok = Token(self.char, TokenType.EXP)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.MUL)
                    self.next()
            case '&':
                if self.peek() == '&':
                    tok = Token(self.char, TokenType.AND)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.BIT_AND)
                    self.next()
            case '=':
                if self.peek() == '=':
                    tok = Token(self.char, TokenType.EQ)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.ASSIGN)
                    self.next()
            case '|':
                if self.peek() == '|':
                    tok = Token(self.char, TokenType.OR)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.BIT_OR)
                    self.next()
            case '<':
                if self.peek() == '=':
                    tok = Token(self.char, TokenType.LTE)
                    self.next()
                elif self.peek() == '<':
                    tok = Token(self.char, TokenType.SHL)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.LT)
                    self.next()
            case '>':
                if self.peek() == '=':
                    tok = Token(self.char, TokenType.GTE)
                    self.next()
                elif self.peek() == '>':
                    tok = Token(self.char, TokenType.SHR)
                    self.next()
                else:
                    tok = Token(self.char, TokenType.GT)
                    self.next()
            case '(':
                tok = Token(self.char, TokenType.LPAREN)
                self.next()
            case ')':
                tok = Token(self.char, TokenType.RPAREN)
                self.next()
            case '[':
                tok = Token(self.char, TokenType.LBRACKET)
                self.next()
            case ']':
                tok = Token(self.char, TokenType.RBRACKET)
                self.next()
            case '{':
                tok = Token(self.char, TokenType.LBRACE)
                self.next()
            case '}':
                tok = Token(self.char, TokenType.RBRACE)
                self.next()
            case ',':
                tok = Token(self.char, TokenType.COMMA)
                self.next()
            case ';':
                tok = Token(self.char, TokenType.SEMICOLON)
                self.next()
            case ':':
                tok = Token(self.char, TokenType.COLON)
                self.next()
            case '.':
                tok = Token(self.char, TokenType.DOT)
                self.next()
            case '~':
                tok = Token(self.char, TokenType.BIT_NOT)
                self.next()
            case '^':
                tok = Token(self.char, TokenType.BIT_XOR)
                self.next()
            case _:
                if self.char.isalpha():
                    tok = self.get_ident()
                    self.next()
                elif self.char.isdigit():
                    tok = self.get_number()
                    self.next()

        if tok.type != TokenType.EOL:
            print(f'{self.line:03d} | ', end='')
            print(tok.type, tok.text, sep=' ' * (32 - len(str(tok.type))))
        else:
            print("----|----")

        if tok.type == None:
            self.abort("unexpected character " + self.char)

        return tok
