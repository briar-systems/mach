# Description

Two issues need to be addressed in the bootstrap compiler:

1. **Codegen Bug**: A crash (SIGSEGV) occurs when running multiple tests that use keyword lexing in the self-hosted lexer. The bug appears to be in the bootstrap compiler's code generation, likely related to:
   - String literal addressing/indexing in the `match_keyword` function
   - Token struct (10 bytes) return handling across function calls
   - Stack/register corruption when the keyword matching code path is executed multiple times

2. **`masm` Keyword Conflict**: Files using `mach.compiler.masm.*` module paths fail to parse because `masm` is lexed as a keyword (`TOKEN_KW_MASM`), and `parser_parse_identifier` only accepts `TOKEN_IDENTIFIER`.

# Outline

## Part 1: Bootstrap Codegen Bug

- [ ] Investigate the crash pattern in `boot/src/compiler/masm/lower.c`:
  - [ ] Review `lower_call` (L909-1214) for small/medium aggregate return handling
  - [ ] Review `lower_stmt` RET handling (L2036-2150) for aggregate returns
  - [ ] Check string literal code generation and addressing
  - [ ] Examine virtual register allocation across function calls
- [ ] Create a minimal reproducer test case
- [ ] Identify and fix the root cause
- [ ] Verify fix by running full test suite
- [ ] Restore full lexer test coverage (remove workarounds from previous session)

## Part 2: `masm` Keyword in Module Paths

Evaluate three options:

### Option A: Rename module paths (masm -> codegen/backend/ir)
- Rename `src/compiler/masm/` directory
- Update all 17+ files with `mach.compiler.masm.*` imports
- Update documentation

### Option B: Change keyword (masm -> asm)
- Update lexer keyword recognition in `boot/src/compiler/lexer.c`
- Update parser keyword handling in `boot/src/compiler/parser.c`
- Update token definitions
- Update ~20 usages of `masm { }` blocks in source files
- Update documentation

### Option C: Allow keywords in use statements
- Modify `parser_parse_identifier` or create `parser_parse_path_segment`
- Accept keywords as valid identifiers in module path contexts
- No user-visible syntax changes needed

- [ ] Analyze each option and select the best approach
- [ ] Implement chosen solution
- [ ] Verify all 13 compile errors are resolved

# Log

## 2026-01-20T22:10 UTC
- Initialized TASK.md for new task session.
- Reviewed context from previous session regarding:
  - Field sorting revert (completed)
  - Crash investigation (partial - root cause identified but not fixed)
  - masm keyword issue (identified but not fixed)
- Current test status: 371 passing, 0 crashed, 13 compile errors (all from `masm` keyword in paths).
- Key files to investigate:
  - `boot/src/compiler/masm/lower.c` - codegen for calls, returns, aggregates
  - `boot/src/compiler/parser.c` - `parser_parse_identifier` and use statement parsing
  - `src/compiler/lexer.mach` - `match_keyword` function (crash trigger)
  - `src/compiler/token.mach` - Token record (10 bytes: pos:i32, len:i32, tag:u8, kind:TokenKind)