[mach](../../README.md) / [front](../README.md) / **parser**

# mach.front.parser

the parser's entry: one token stream parsed into a syntax tree, handed back only when
the parse completed. its docstrings are linted by name resolution, for the project's
own modules only

The directory's root module is [`mach.front.parser`](../parser.md).

| Module | |
| :--- | :--- |
| [`grammar`](grammar.md) | the grammar: a recursive descent over a token stream, one function per production, each pushing its node into the tree through the parser state |
| [`iasm`](iasm.md) | the parse of an inline `asm` statement |
| [`state`](state.md) | the parser's state: its cursor over the tokens, the tree it builds and what it reports |

