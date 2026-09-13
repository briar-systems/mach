# mach.lang.fe.parser.state

## def ParseFailKind

```mach
pub def ParseFailKind: u8
```

## val PARSE_FAIL_OOM

```mach
pub val PARSE_FAIL_OOM:   ParseFailKind = 0
```

## val PARSE_FAIL_ICE

```mach
pub val PARSE_FAIL_ICE:   ParseFailKind = 1
```

## val PARSE_FAIL_DIAG

```mach
pub val PARSE_FAIL_DIAG:  ParseFailKind = 2
```

## val PARSE_FAIL_DEPTH

```mach
pub val PARSE_FAIL_DEPTH: ParseFailKind = 3
```

## rec ParseFail

```mach
pub rec ParseFail;
```

## rec Parser

```mach
pub rec Parser;
```

## rec PendingDiag

```mach
pub rec PendingDiag;
```

what a deciding walk said. the walk that decides between the two readings of
`name[` cannot speak as it goes: if its reading loses, nothing it said was
ever true. it records here, and whichever reading becomes the parse says what
it recorded, in order.

## val MAX_NEST_DEPTH

```mach
pub val MAX_NEST_DEPTH: usize = 2048
```

the deepest nesting the parser will descend into. this is the last-resort
stack guard, not the semantic limit: the recursive-type checker refuses at
1024 (REACHES_MAX_DEPTH in fe/sema/infer.mach) with a message that says what
to do about it, so the parser must sit above that or sema never gets to speak.
the compiler's own source and the standard library reach 14.

## rec ListBuf

```mach
pub rec ListBuf[T];
```

## fun make

```mach
pub fun make(tokens: *lexer.TokenStream, out: *ast.Ast, diags: *diagnostic.DiagnosticStore) Parser;
```

## fun current

```mach
pub fun current(p: *Parser) token.Token;
```

## fun peek

```mach
pub fun peek(p: *Parser, offset: usize) token.Token;
```

## fun advance

```mach
pub fun advance(p: *Parser) token.Token;
```

## fun at

```mach
pub fun at(p: *Parser, kind: token.Kind) bool;
```

## fun at_kw

```mach
pub fun at_kw(p: *Parser, kw: str) bool;
```

## fun at_eof

```mach
pub fun at_eof(p: *Parser) bool;
```

## fun at_decl_start

```mach
pub fun at_decl_start(p: *Parser) bool;
```

## fun eat

```mach
pub fun eat(p: *Parser, kind: token.Kind) bool;
```

## fun eat_kw

```mach
pub fun eat_kw(p: *Parser, kw: str) bool;
```

## fun expect

```mach
pub fun expect(p: *Parser, kind: token.Kind, message: str) bool;
```

## fun previous

```mach
pub fun previous(p: *Parser) token.Token;
```

## fun expect_end

```mach
pub fun expect_end(p: *Parser, kind: token.Kind, message: str) token.Token;
```

## fun failure

```mach
pub fun failure(f: ParseFail) fail.Fail;
```

## fun diag_mark

```mach
pub fun diag_mark(p: *Parser) usize;
```

## fun diag_discard

```mach
pub fun diag_discard(p: *Parser, m: usize);
```

## fun diag_flush

```mach
pub fun diag_flush(p: *Parser, m: usize);
```

the reading that recorded these is the parse, so they are real. inside an
enclosing deciding walk they stay in the buffer, because that walk may yet
lose and take them with it; at the top they go to the sink, in order.

## fun diag_dnit

```mach
pub fun diag_dnit(p: *Parser, alloc: *A.Allocator);
```

## fun error_at_current

```mach
pub fun error_at_current(p: *Parser, message: str);
```

## fun error_recover_advance

```mach
pub fun error_recover_advance(p: *Parser, message: str);
```

## fun error_at

```mach
pub fun error_at(p: *Parser, span: token.Span, message: str);
```

## fun fatal_oom_at

```mach
pub fun fatal_oom_at(p: *Parser, span: token.Span);
```

## fun fatal_oom

```mach
pub fun fatal_oom(p: *Parser);
```

## fun fatal_ice_at

```mach
pub fun fatal_ice_at(p: *Parser, span: token.Span, message: str);
```

## fun probed_init

```mach
pub fun probed_init(p: *Parser, alloc: *A.Allocator);
```

a token position where the index reading of `name[` has already been tried
and did not parse. the verdict is about tokens alone, so it survives any
rollback, and without it a malformed chain re-derives the same failure at
every level and doubles with depth.

## fun probed_dnit

```mach
pub fun probed_dnit(p: *Parser, alloc: *A.Allocator);
```

## fun index_reading_failed_here

```mach
pub fun index_reading_failed_here(p: *Parser, at: usize) bool;
```

## fun record_index_reading_failed

```mach
pub fun record_index_reading_failed(p: *Parser, at: usize);
```

## fun descend

```mach
pub fun descend(p: *Parser) bool;
```

## fun ascend

```mach
pub fun ascend(p: *Parser);
```

## fun ast_mark

```mach
pub fun ast_mark(p: *Parser) ast.AstMark;
```

## fun ast_rollback

```mach
pub fun ast_rollback(p: *Parser, m: ast.AstMark);
```

## fun sync_to

```mach
pub fun sync_to(p: *Parser, k0: token.Kind, k1: token.Kind, k2: token.Kind);
```

## fun sync_to_body

```mach
pub fun sync_to_body(p: *Parser, k0: token.Kind, k1: token.Kind, k2: token.Kind);
```

## fun sync_to_decl

```mach
pub fun sync_to_decl(p: *Parser);
```

## fun span_of

```mach
pub fun span_of(start: token.Span, end: token.Span) token.Span;
```

## fun push_expr

```mach
pub fun push_expr(p: *Parser, e: expr.Expr) id.ExprId;
```

## fun push_stmt

```mach
pub fun push_stmt(p: *Parser, s: stmt.Stmt) id.StmtId;
```

## fun push_decl

```mach
pub fun push_decl(p: *Parser, d: decl.Decl) id.DeclId;
```

## fun push_type

```mach
pub fun push_type(p: *Parser, t: type.Type) id.TypeId;
```

## fun push_module

```mach
pub fun push_module(p: *Parser, m: module.Module) id.ModuleNodeId;
```

## fun push_decl_id

```mach
pub fun push_decl_id(p: *Parser, id_: id.DeclId) u32;
```

## fun push_stmt_id

```mach
pub fun push_stmt_id(p: *Parser, id_: id.StmtId) u32;
```

## fun push_expr_id

```mach
pub fun push_expr_id(p: *Parser, id_: id.ExprId) u32;
```

## fun push_type_id

```mach
pub fun push_type_id(p: *Parser, id_: id.TypeId) u32;
```

## fun list_buf_init

```mach
pub fun list_buf_init[T]() ListBuf[T];
```

## fun list_buf_push

```mach
pub fun list_buf_push[T](p: *Parser, b: *ListBuf[T], x: T);
```

## fun list_buf_free

```mach
pub fun list_buf_free[T](p: *Parser, b: *ListBuf[T]);
```

## fun decl_id_buf_flush

```mach
pub fun decl_id_buf_flush(p: *Parser, b: *ListBuf[id.DeclId], out_len: *u32) u32;
```

## fun stmt_id_buf_flush

```mach
pub fun stmt_id_buf_flush(p: *Parser, b: *ListBuf[id.StmtId], out_len: *u32) u32;
```

## fun expr_id_buf_flush

```mach
pub fun expr_id_buf_flush(p: *Parser, b: *ListBuf[id.ExprId], out_len: *u32) u32;
```

## fun type_id_buf_flush

```mach
pub fun type_id_buf_flush(p: *Parser, b: *ListBuf[id.TypeId], out_len: *u32) u32;
```

## fun field_init_buf_flush

```mach
pub fun field_init_buf_flush(p: *Parser, b: *ListBuf[expr.FieldInit], out_len: *u32) u32;
```

## fun typed_name_buf_flush

```mach
pub fun typed_name_buf_flush(p: *Parser, b: *ListBuf[decl.TypedName], out_len: *u32) u32;
```

## fun tag_case_buf_flush

```mach
pub fun tag_case_buf_flush(p: *Parser, b: *ListBuf[decl.TagCase], out_len: *u32) u32;
```

## fun comptime_branch_buf_flush

```mach
pub fun comptime_branch_buf_flush(p: *Parser, b: *ListBuf[decl.ComptimeBranch], out_len: *u32) u32;
```

