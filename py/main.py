import sys

from mach.lex import *
from mach.ast import *
from mach.tok import *

def test_lex():
    for t, v in tokens.items():
        lex = Lex(v)
        tok = lex.next_tok()
        
        if tok.text == "\n": tok.text = '\\n'
        if tok.text == "\0": tok.text = '\\0'

        ok = tok.type == t
        print("OK" if ok else "--", "|", end=" ")
        print(tok.type, tok.text, sep=' ' * (20 - len(str(tok.type))))

def main():
    # test_lex()

    if len(sys.argv) != 2:
        print("usage: mach <file>")
        sys.exit(1)
    
    with open(sys.argv[1]) as f:
        src = f.read()
        lex = Lex(src)
        ast = None
        try:
            ast = Parser(lex).parse()
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
