[mach](../README.md) / **front**

# mach.front

the front end: the lexer, the parser and the syntax tree, module loading and name
resolution, comptime evaluation, decorators, type checking, and doc comments

| Module | |
| :--- | :--- |
| [`asm`](asm.md) | the lexis of an inline-asm body that the language owns: its `#` comments, and the `{name}` references that bind a local |
| [`ast`](ast.md) | the syntax tree of one parsed file |
| [`comptime`](comptime.md) | the comptime evaluator: an expression's value under the capabilities of the phase that asks, over the scope it runs in |
| [`decorator`](decorator.md) | the decorator registry: one row per decorator the language knows, which every phase reads by id |
| [`doc`](doc.md) | the model of one doc comment: its prose paragraphs, the summary first, and the component lines after `# ---` |
| [`doclint`](doclint.md) | the docstring linter: the component lines of every documented `pub fun`, `pub rec`, `pub uni` and `pub tag` checked against the declaration they document |
| [`expect`](expect.md) | `#[expect("<key>", ...)]`: a declaration's acknowledgement of named warnings |
| [`lexer`](lexer.md) | the lexer: a file's source as a token stream, with its lexical errors and the runs of line comments that documentation is read from |
| [`lexis`](lexis.md) | the reserved words and identifier characters the lexer, the parser and the migrator share |
| [`literal`](literal.md) | scanning and decoding a literal's spelling: the value an integer, float, character or string literal names and the suffix it carries, or why it does not scan |
| [`load`](load.md) | module loading: every module a build reaches, its gates decided and its imports bound |
| [`parser`](parser.md) | the parser's entry: one token stream parsed into a syntax tree, handed back only when the parse completed |
| [`path`](path.md) | dotted paths read from the source, with the trivia between their segments skipped |
| [`refusal`](refusal.md) | a module refused by a module-scope `$error` |
| [`resolve`](resolve.md) | name resolution: every name a module spells bound to the symbol it means |
| [`token`](token.md) | the tokens of the language: their kinds, fixed punctuation spellings and infix precedence |
| [`transition`](transition.md) | the forms of the previous major and the tables that carry a project across them |

| Directory | |
| :--- | :--- |
| [`ast/`](ast/README.md) | the syntax tree of one parsed file |
| [`comptime/`](comptime/README.md) | the comptime evaluator: an expression's value under the capabilities of the phase that asks, over the scope it runs in |
| [`parser/`](parser/README.md) | the parser's entry: one token stream parsed into a syntax tree, handed back only when the parse completed |
| [`sema/`](sema/README.md) |  |

