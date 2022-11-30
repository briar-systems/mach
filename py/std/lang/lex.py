from . import tok
from . import err


class Lex:
    def __init__(self, src):
        self.src = src
        self.pos = 0

        # make sure the source ends with an EOF token
        if self.src[-1] != '\0':
            self.src += '\0'

    def val(self) -> str:
        return self.src[self.pos]

    def next(self):
        if self.pos < len(self.src):
            self.pos += 1

    def peek(self):
        if self.pos < len(self.src):
            return self.src[self.pos+1]

        return '\0'

    def check(self, val, str):
        if val == str:
            return False
        if val == '\0':
            return False

        return True

    def get_from(self, beg, end=None):
        return self.src[beg:end or self.pos + 1]

    def next_tok(self) -> tok.Token:
        # snag comments
        if self.val() == '#':
            self.next()
            beg = self.pos
            while self.check(self.peek(), '\n'):
                self.next()

            return tok.Token(tok.COMMENT, beg, self.get_from(beg))

        if self.val() in ",":
            val = self.val()

        # identifier
        # checks identifiers against keywords
        if self.val() in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_":
            beg = self.pos
            while self.peek().isalnum():
                self.next()

            val = self.get_from(beg)
            res = tok.find(val)
            if res is not None:
                return tok.Token(res, beg, val)

            return tok.Token(tok.IDENT, beg, val)

        # numbers
        if self.val() in "0123456789":
            beg = self.pos

            # collect leading digits
            while self.peek().isdigit():
                self.next()

            # check for classifying characters
            match self.val():
                case '.':
                    self.next()
                    while self.val() in '0123456789_':
                        self.next()

                    return tok.Token(tok.LIT_FLOAT, beg, self.get_from(beg))
                case 'x':
                    self.next()
                    while self.peek() in '0123456789abcdefABCDEF_':
                        self.next()
                case 'b':
                    self.next()
                    while self.peek() in '01_':
                        self.next()
                case 'o':
                    self.next()
                    while self.peek() in '01234567_':
                        self.next()

            return tok.Token(tok.LIT_INT, beg, self.get_from(beg))

        # string literals
        if self.val() == '"':
            self.next()
            beg = self.pos
            while self.check(self.peek(), '"'):
                self.next()

            self.next()
            return tok.Token(tok.LIT_STR, beg, self.get_from(beg, self.pos))

        # character literals
        if self.val() == "'":
            self.next()
            beg = self.pos
            while self.check(self.peek(), "'"):
                self.next()

            self.next()
            return tok.Token(tok.LIT_CHAR, beg, self.get_from(beg, self.pos))

        # catch all single character tokens
        val = self.val()
        res = tok.find(val)
        if res is not None:
            return tok.Token(res, self.pos, val)

        # double character token check
        two = self.val() + self.peek()
        res = tok.find(two)
        if res is not None:
            return tok.Token(res, self.pos, two)

        # if we get here, we have an error
        raise Exception(f'Unexpected character: {self.val()}')

    def exec(self):
        tokens = []
        while True:
            # skip whitespace
            while self.val() in ' \t\r\n':
                self.next()

            try:
                next = self.next_tok()
            except Exception as e:
                err.add(str(e), 'lex', self.pos)
                return tokens

            tokens.append(next)

            if next.kind == tok.EOF:
                break

            self.next()

        return tokens
