import sys

from mach.lex import *
from mach.ast import *
from mach.tok import *


def test_lex():
    for t, v in tokens.items():
        lex = Lex(v)
        tok = lex.next_tok()

        if tok.val == "\n":
            tok.val = '\\n'
        if tok.val == "\0":
            tok.val = '\\0'

        ok = tok.type == t
        print("OK" if ok else "--", "|", end=" ")
        print(tok.type, tok.val, sep=' ' * (20 - len(str(tok.type))))


def main():
    # test_lex()

    if len(sys.argv) != 2:
        print("usage: mach <file>")
        sys.exit(1)

    with open(sys.argv[1]) as f:
        src = f.read()
        lex = Lex(src)
        par = Parser(lex)
        ast = None
        try:
            ast = par.exec()
        except InvalidTokenException as e:
            print(e)
            sys.exit(1)


if __name__ == "__main__":
    main()

# if tok.type != TokenType.EOL:
#     print(f'{self.line:03d} | ', end='')
#     print(tok.type, tok.text, sep=' ' * (32 - len(str(tok.type))))
# else:
#     print("----|----")
