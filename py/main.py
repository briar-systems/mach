import sys

from mach.lex import *
from mach.ast import *
from mach.tok import *
from mach import errors


def test_lex():
    for t, v in tokens.items():
        lex = Lex(v)
        tok = lex.next_tok()

        if tok.val == "\n":
            tok.val = '\\n'
        if tok.val == "\0":
            tok.val = '\\0'

        ok = tok.type == t
        print("OK" if ok else "!!", "|", end=" ")
        print(t, tok.type, tok.val, sep=' ' * (20 - len(str(t))))


def main():
    # test_lex()

    if len(sys.argv) != 2:
        print("usage: mach <file>")
        sys.exit(1)

    # read source file
    src = open(sys.argv[1]).read()

    # tokenize
    lex = Lex(src)
    tokens = lex.exec()

    # pprint tokens
    print("ROW:COL#LEN NAME                 `VAL`")
    print("--------------------------------------")
    for tok in tokens:
        row, col = get_row_col(src, tok.pos)
        print(f'{row:03}:{col:03}#{len(tok.val):03} {tok.kind.name:20} `{tok.val}`')

    print("--------------------------------------")
    print("ERRORS:")
    errors.print_errors()    

    # parse
    # par = Parser(tokens)
    # ast = par.exec()


if __name__ == "__main__":
    main()

# if tok.type != TokenType.EOL:
#     print(f'{self.line:03d} | ', end='')
#     print(tok.type, tok.text, sep=' ' * (32 - len(str(tok.type))))
# else:
#     print("----|----")
