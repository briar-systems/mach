import sys

from mach.lex import *
from mach.ast import *

def main():
    if len(sys.argv) != 2:
        print("usage: mach <file>")
        sys.exit(1)
    
    with open(sys.argv[1]) as f:
        src = f.read()
        lex = Lex(src)
        ast = Parser(lex).parse()

if __name__ == "__main__":
    main()
