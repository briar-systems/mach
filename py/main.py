import sys

from std.lang import tok
from std.lang import lex
from std.lang import err


def main():
    # test_lex()

    if len(sys.argv) != 2:
        print("usage: mach <file>")
        sys.exit(1)

    # read source file
    src = open(sys.argv[1]).read()

    # tokenize
    lexer = lex.Lex(src)
    tokens = lexer.exec()

    # pprint tokens
    print("ROW:COL#LEN NAME                 `VAL`")
    print("--------------------------------------")
    for t in tokens:
        row, col = tok.get_row_col(src, t.pos)
        print(f'{row:03}:{col:03}#{len(t.val):03} {t.kind.name:20} `{t.val}`')

    print("--------------------------------------")
    print("ERRORS:")
    err.print_errors()    

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
