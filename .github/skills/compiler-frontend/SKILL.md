---
name: compiler-frontend
description: Edits the Mach compiler frontend in C (tokens, lexer, parser, AST). Use when adding syntax, fixing parsing bugs, or improving diagnostics in boot/include/compiler and boot/src/compiler.
---

# compiler frontend (lexer/parser/ast) skill

## when to use

Use this skill when you are working on:

- lexical rules and tokenization
- parser grammar / precedence / statement forms
- AST node shapes and construction
- syntax-level diagnostics (parser errors)

If the request changes language syntax/semantics (keywords, type rules, coercions, control-flow semantics), stop and confirm intent first.

## file map

- tokens: `boot/include/compiler/token.h`, `boot/src/compiler/token.c`
- lexer: `boot/include/compiler/lexer.h`, `boot/src/compiler/lexer.c`
- parser: `boot/include/compiler/parser.h`, `boot/src/compiler/parser.c`
- ast: `boot/include/compiler/ast.h`, `boot/src/compiler/ast.c`

Helpful reference docs (source-of-truth for language behavior):
- `doc/cheatsheet.md`
- `doc/keywords.md`, `doc/expressions.md`, `doc/types.md`, `doc/modules.md`, `doc/quirks.md`

## safe workflow for syntax changes

1. identify whether the change is purely a bug fix vs a language design change.
   - if it changes user-visible syntax/semantics: pause and discuss.
2. update frontend in this order (small commits):
   1) `token.*`: add token kinds/printing helpers if needed
   2) `lexer.*`: recognize the new lexeme(s)
   3) `ast.*`: add node kinds/fields/constructors
   4) `parser.*`: parse into the new AST shape
   5) `sema.*`: enforce the rule (see `compiler-sema-and-modules` concerns inside `boot/src/compiler/sema.c`)
   6) update docs under `doc/` to match
3. rebuild the bootstrap compiler and compile a tiny repro program.

## parser guidelines

- prefer explicit, readable parsing functions (`parser_parse_stmt_*`, `parser_parse_expr_*`).
- preserve existing precedence levels (`Precedence` enum in `boot/include/compiler/parser.h`).
- when introducing new grammar forms, ensure error recovery (`parser_synchronize`) still works.
- diagnostics should report:
  - the offending token
  - the expected form
  - enough context to fix the code (avoid generic "failed" messages)

## common edge cases

- module path inference and `use` statements can create surprising parse order dependencies; keep `use` parsing strict and clear.
- typed literals and casts (`value::Type`, `Type{...}`) often create ambiguity. tighten the parser before loosening.
- keep literal coercion rules consistent with docs: coercion only at declaration point; explicit casts elsewhere.
