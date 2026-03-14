# Mach Compiler Design

A self-hosted compiler for the Mach language, built on `mach-std` and `mach-masm`.

Designed as a **library with a CLI**, not a CLI with internals. Every subsystem is independently importable, testable, and replaceable. The architecture assumes multi-target compilation, structured diagnostics for tooling integration, and per-function parallel codegen.

## Principles

- **Index, don't point.** All cross-references use typed u32 IDs into flat tables. No void pointers, no unsafe casts. Tables own data; IDs are cheap copies.
- **Immutable AST.** Parsing produces a frozen `NodeStore`. Semantic analysis writes to separate side tables keyed by `NodeId`, never mutates nodes.
- **Explicit phases.** Each compilation phase has typed inputs and outputs. No implicit shared mutable state between phases.
- **Structured diagnostics.** Errors carry machine-readable source spans, severity, and context. The diagnostic system is self-contained — no dependency on sema or module internals.
- **Target-independent by default.** ISA-specific code lives exclusively behind the `ISA` interface. Register allocation, the optimization pipeline, and IR lowering are target-agnostic.
- **Parallel-ready.** Module loading/parsing is independent per file. Per-function codegen (lower → ISel → regalloc → encode) is embarrassingly parallel. No shared mutable state between compilation units.
- **Naming convention.** Lifecycle: `init`/`dnit`. Multi-type modules prefix functions with the type name (`buf_init`, `buf_dnit`). Single-primary-type modules use bare names. Constructors that create child entries in a table: `create`. All strings that cross module boundaries are interned in the session `StringPool`.

---

## Compilation Pipeline

```
Source files
    │
    ├─ per file, parallel ──────────────────────────┐
    │   Lex → TokenStream                           │
    │   Parse → NodeStore (immutable AST)            │
    │   Discover imports (eval $if target predicates)│
    └───────────────────────────────────────────────┘
    │
    ▼
Module Graph (dependency DAG, topologically sorted)
    │
    ├─ per module, dependency order ────────────────┐
    │   Phase 1: Collect (register symbol names)     │
    │   Phase 2: Resolve (type shells, layouts)      │
    │   Phase 3: Check (body types, generics)        │
    │   Phase 4: Validate (correctness)              │
    └───────────────────────────────────────────────┘
    │
    ▼
Typed Module Set (AST + TypeTable + SymbolTable + side tables)
    │
    ├─ per function, parallel ──────────────────────┐
    │   Lower → masm.Function (target-independent)   │
    │   Verify (masm.verify)                         │
    │   Optimize (DCE, const fold — optional)        │
    │   Build CFG + Liveness (masm.cfg)              │
    │   ISel → InstBuf                               │
    │   RegAlloc → InstBuf (physical regs)           │
    │   Encode → object section bytes                │
    └───────────────────────────────────────────────┘
    │
    ▼
Object files (.o)
    │
    ▼
Link → Executable
```

---

## File Layout

```
src/
├── main.mach                        # CLI entry, dispatches to cmd/
├── cli/
│   └── args.mach                    # generic argument parsing utilities
├── cmd/
│   ├── build.mach                   # build command
│   ├── run.mach                     # run command
│   ├── test.mach                    # test runner
│   ├── dep.mach                     # dependency management + git submodule ops
│   ├── init.mach                    # project scaffolding
│   └── help.mach                    # help text
├── config/
│   └── project.mach                 # mach.toml loading, DepConfig, target config
│
├── compiler/
│   ├── compiler.mach                # public API surface (compile_project, compile_file)
│   ├── session.mach                 # compilation session, StringPool, CompileOpts
│   ├── diag.mach                    # structured diagnostics (self-contained)
│   │
│   ├── lex/
│   │   ├── token.mach               # Tag, Token, Span
│   │   └── lexer.mach               # tokenizer, TokenStream
│   │
│   ├── parse/
│   │   ├── ast.mach                 # NodeKind, Node, NodeStore, accessors
│   │   └── parser.mach              # recursive descent + Pratt parser
│   │
│   ├── sema/
│   │   ├── sema.mach                # analyzer context, 4-phase driver
│   │   ├── scope.mach               # SymbolId, ScopeId, Symbol, Scope, SymbolTable
│   │   ├── type.mach                # TypeId, TypeKind, Type, TypeTable, layout
│   │   ├── module.mach              # ModuleId, Module, ModuleTable, dependency graph
│   │   ├── resolve.mach             # name resolution, type resolution (phases 1-2)
│   │   ├── check.mach               # body type-checking, validation (phases 3-4)
│   │   ├── mono.mach                # monomorphization cache, AST cloning
│   │   └── comptime.mach            # compile-time evaluation ($if, $size_of, etc.)
│   │
│   ├── lower/
│   │   ├── lower.mach               # LowerContext, per-module driver
│   │   ├── expr.mach                # expression → MASM IR
│   │   ├── stmt.mach                # statement → MASM IR
│   │   └── data.mach                # globals, string constants, data declarations
│   │
│   ├── opt/
│   │   ├── pass.mach                # PassPipeline, pass registration, pipeline builder
│   │   ├── dce.mach                 # dead code elimination on MASM IR
│   │   └── constfold.mach           # constant folding on MASM IR
│   │
│   ├── codegen.mach                 # per-function codegen orchestrator
│   ├── regalloc.mach                # target-independent linear scan register allocator
│   ├── linker.mach                  # static linker (section merge, reloc, emit)
│   │
│   └── target/
│       ├── target.mach              # Target composition (ISA + ABI + OS + ObjectFormat)
│       ├── os.mach                  # OS config (entry point, base addr, page size)
│       ├── abi.mach                 # ABI interface (classify, register files, frame)
│       ├── abi/
│       │   └── sysv64.mach          # System V AMD64 ABI
│       ├── isa.mach                 # ISA interface (Inst, InstBuf, Operand, ISel, encode)
│       ├── isa/
│       │   └── x86_64/
│       │       ├── defs.mach        # register IDs, opcodes, encoding flags
│       │       ├── isel.mach        # MASM IR → isa.Inst
│       │       └── encode.mach      # isa.Inst → bytes
│       ├── of.mach                  # object format interface (Section, ObjWriter, ExecFormat)
│       └── of/
│           └── elf.mach             # ELF object + executable emission
```

---

## CLI Surface

Commands: `build`, `run`, `test`, `dep` (list/info/add/del/pull/tidy), `init`, `help`

Build flags: `--target <name>`, `-o <file>`, `-m <path>`, `-I prefix=dir`, `--release`, `--emit-masm`, `--emit-asm`, `--verbose`, `-j <N>` (parallelism)

Test flags: `--test <pattern>`, `--module <pattern>`, `-v`

Project config: `mach.toml` with `[project]`, `[targets.<name>]`, `[deps.<name>]` sections.

---

## Module Specifications

### `compiler/session.mach` — Compilation Session

Owns all shared compilation state and the global `StringPool`. Phase methods drive the pipeline. Query methods work after any phase — external tools (LSP, formatter) can stop after `analyze()` and inspect types/symbols without codegen.

```
pub rec CompileOpts {
    release:    bool;
    emit_masm:  bool;
    emit_asm:   bool;
    verbose:    bool;
    verify_ir:  bool;
    opt_level:  u8;         # 0 = none, 1 = basic (DCE + constfold), 2 = full
    jobs:       u32;        # parallel codegen workers (0 = auto)
}

pub rec CompileResult {
    ok:         bool;
    obj_paths:  *str;
    obj_count:  i32;
    binary:     str;        # final executable path (nil if link skipped)
}

pub rec StringPool {
    alloc:   &allocator.Allocator;
    strings: *str;
    count:   u32;
    cap:     u32;
    dedup:   map.Map[str, u32];
}

pub rec Session {
    alloc:   &allocator.Allocator;
    strings: StringPool;
    diag:    diag.DiagList;
    modules: module.ModuleTable;
    symbols: scope.SymbolTable;
    types:   type.TypeTable;
    mono:    mono.MonoCache;
    target:  *target.Target;
    opts:    CompileOpts;
}

pub fun init(alloc: &allocator.Allocator, opts: CompileOpts) Session;
pub fun dnit(s: *Session);

pub fun pool_intern(pool: *StringPool, text: str) str;

# pipeline phases
pub fun load(s: *Session, root_path: str, src_dir: str, dep_dir: str, project_id: str);
pub fun analyze(s: *Session);
pub fun lower(s: *Session) masm.module.Module;
pub fun codegen(s: *Session, m: *masm.module.Module) CompileResult;
pub fun link(s: *Session, obj_paths: *str, obj_count: i32, output: str) linker.LinkResult;

# queries (valid after any phase that populates the relevant table)
pub fun diagnostics(s: &Session) &diag.DiagList;
pub fun get_type(s: &Session, id: type.TypeId) &type.Type;
pub fun get_symbol(s: &Session, id: scope.SymbolId) &scope.Symbol;
pub fun node_type(s: &Session, mod_id: module.ModuleId, node_id: ast.NodeId) type.TypeId;
```

---

### `compiler/compiler.mach` — Public API

Convenience entry points. These create a `Session`, run the full pipeline, and return the result.

```
pub fun compile_project(alloc: &allocator.Allocator, project_path: str,
                        target_name: str, opts: session.CompileOpts) session.CompileResult;

pub fun compile_file(alloc: &allocator.Allocator, source_path: str,
                     output_path: str, opts: session.CompileOpts) session.CompileResult;
```

---

### `compiler/diag.mach` — Structured Diagnostics

Self-contained — no dependency on sema, module, or session internals.

```
pub def Severity: u8;
pub val SEV_ERROR:   Severity = 0;
pub val SEV_WARNING: Severity = 1;
pub val SEV_NOTE:    Severity = 2;
pub val SEV_HINT:    Severity = 3;

pub rec SourceLoc {
    file:   str;
    line:   u32;
    col:    u32;
    offset: u32;
    len:    u16;
}

pub rec Diagnostic {
    severity: Severity;
    loc:      SourceLoc;
    message:  str;
    notes:    *Diagnostic;
}

pub rec DiagList {
    alloc:     &allocator.Allocator;
    items:     *Diagnostic;
    count:     u32;
    cap:       u32;
    err_count: u32;
}

pub fun init(alloc: &allocator.Allocator) DiagList;
pub fun dnit(dl: *DiagList);
pub fun emit(dl: *DiagList, sev: Severity, loc: SourceLoc, msg: str);
pub fun emit_note(dl: *DiagList, parent: *Diagnostic, loc: SourceLoc, msg: str);
pub fun error(dl: *DiagList, loc: SourceLoc, msg: str);
pub fun warning(dl: *DiagList, loc: SourceLoc, msg: str);
pub fun had_errors(dl: &DiagList) bool;
pub fun error_count(dl: &DiagList) u32;

# source_fn: callback to retrieve source text for a file path
pub def SourceFn: fun(str) str;
pub fun print_terminal(dl: &DiagList, get_source: SourceFn);
pub fun print_json(dl: &DiagList, writer: &io.Writer);
```

---

### `compiler/lex/token.mach` — Token Definitions

```
pub def Tag: u8;

pub val TAG_IDENT:       Tag = 0;
pub val TAG_LIT_INT:     Tag = 1;
pub val TAG_LIT_FLOAT:   Tag = 2;
pub val TAG_LIT_CHAR:    Tag = 3;
pub val TAG_LIT_STRING:  Tag = 4;

pub val TAG_LPAREN: Tag = 5;  pub val TAG_RPAREN: Tag = 6;
pub val TAG_LBRACE: Tag = 7;  pub val TAG_RBRACE: Tag = 8;
pub val TAG_LBRACKET: Tag = 9; pub val TAG_RBRACKET: Tag = 10;

pub val TAG_SEMI: Tag = 11;       pub val TAG_COLON: Tag = 12;
pub val TAG_COMMA: Tag = 13;      pub val TAG_DOT: Tag = 14;
pub val TAG_QUESTION: Tag = 15;   pub val TAG_AT: Tag = 16;
pub val TAG_DOLLAR: Tag = 17;     pub val TAG_HASH: Tag = 18;
pub val TAG_AMP: Tag = 19;        pub val TAG_STAR: Tag = 20;

pub val TAG_PLUS: Tag = 21;       pub val TAG_MINUS: Tag = 22;
pub val TAG_SLASH: Tag = 23;      pub val TAG_PERCENT: Tag = 24;
pub val TAG_EQ: Tag = 25;         pub val TAG_EQ_EQ: Tag = 26;
pub val TAG_BANG: Tag = 27;       pub val TAG_BANG_EQ: Tag = 28;
pub val TAG_LT: Tag = 29;        pub val TAG_LT_EQ: Tag = 30;
pub val TAG_LT_LT: Tag = 31;     pub val TAG_GT: Tag = 32;
pub val TAG_GT_EQ: Tag = 33;     pub val TAG_GT_GT: Tag = 34;
pub val TAG_AMP_AMP: Tag = 35;   pub val TAG_PIPE: Tag = 36;
pub val TAG_PIPE_PIPE: Tag = 37; pub val TAG_CARET: Tag = 38;
pub val TAG_TILDE: Tag = 39;     pub val TAG_COLON_COLON: Tag = 40;
pub val TAG_DOT_DOT_DOT: Tag = 41;

pub val TAG_EOF:   Tag = 254;
pub val TAG_ERROR: Tag = 255;

pub rec Span { offset: u32; len: u16; }
pub rec Token { span: Span; tag: Tag; }

pub fun make(tag: Tag, offset: u32, len: u16) Token;
pub fun infix_precedence(tag: Tag) u8;
pub fun is_right_assoc(tag: Tag) bool;
pub fun tag_str(tag: Tag) str;
```

Keywords are all lexed as `TAG_IDENT`. The parser distinguishes them by string comparison.

---

### `compiler/lex/lexer.mach` — Tokenizer

```
pub rec TokenStream {
    tokens: *token.Token;
    count:  u32;
    source: str;
}

pub fun tokenize(source: str, alloc: &allocator.Allocator) TokenStream;
pub fun text(stream: &TokenStream, tok: token.Token) str;
pub fun line_of(source: str, offset: u32) u32;
pub fun col_of(source: str, offset: u32) u32;
```

---

### `compiler/parse/ast.mach` — AST Nodes

Flat arena-style storage. Nodes are 16 bytes (`kind:u8 + tok_idx:u32 + end_tok:u32 + payload:u32`). Storing both the start and end token gives full source ranges for diagnostics and LSP integration. The payload indexes into a `u32` extra array for kind-specific data. Variable-length children are stored as `NodeSpan {start, len}` in extra. Strings are interned per-store; names that cross module boundaries are promoted to the session `StringPool`.

```
pub def NodeId: u32;
pub val NODE_NIL: NodeId = 0xFFFFFFFF;
pub def StrId: u32;
pub val STR_NIL: StrId = 0xFFFFFFFF;
pub def NodeKind: u8;

# 0: structural
pub val NK_PROGRAM: NodeKind = 0;

# 1-11: declarations
pub val NK_DECL_USE:  NodeKind = 1;   pub val NK_DECL_PUB: NodeKind = 2;
pub val NK_DECL_EXT:  NodeKind = 3;   pub val NK_DECL_DEF: NodeKind = 4;
pub val NK_DECL_REC:  NodeKind = 5;   pub val NK_DECL_UNI: NodeKind = 6;
pub val NK_DECL_VAL:  NodeKind = 7;   pub val NK_DECL_VAR: NodeKind = 8;
pub val NK_DECL_FUN:  NodeKind = 9;   pub val NK_DECL_TEST: NodeKind = 10;
pub val NK_DECL_FWD:  NodeKind = 11;

# 12-21: statements
pub val NK_STMT_EXPR:  NodeKind = 12; pub val NK_STMT_BLOCK: NodeKind = 13;
pub val NK_STMT_IF:    NodeKind = 14; pub val NK_STMT_OR: NodeKind = 15;
pub val NK_STMT_FOR:   NodeKind = 16; pub val NK_STMT_RET: NodeKind = 17;
pub val NK_STMT_BRK:   NodeKind = 18; pub val NK_STMT_CNT: NodeKind = 19;
pub val NK_STMT_FIN:   NodeKind = 20; pub val NK_STMT_ASM: NodeKind = 21;

# 22-24: comptime
pub val NK_COMPTIME_IF: NodeKind = 22;
pub val NK_COMPTIME_OR: NodeKind = 23;
pub val NK_COMPTIME:    NodeKind = 24;

# 25-42: expressions
pub val NK_EXPR_IDENT:      NodeKind = 25;
pub val NK_EXPR_LIT_INT:    NodeKind = 26;
pub val NK_EXPR_LIT_FLOAT:  NodeKind = 27;
pub val NK_EXPR_LIT_CHAR:   NodeKind = 28;
pub val NK_EXPR_LIT_STR:    NodeKind = 29;
pub val NK_EXPR_LIT_BOOL:   NodeKind = 30;
pub val NK_EXPR_LIT_NIL:    NodeKind = 31;
pub val NK_EXPR_BINARY:     NodeKind = 32;
pub val NK_EXPR_UNARY:      NodeKind = 33;
pub val NK_EXPR_CALL:       NodeKind = 34;
pub val NK_EXPR_INDEX:      NodeKind = 35;
pub val NK_EXPR_FIELD:      NodeKind = 36;
pub val NK_EXPR_CAST:       NodeKind = 37;
pub val NK_EXPR_ADDR:       NodeKind = 38;
pub val NK_EXPR_DEREF:      NodeKind = 39;
pub val NK_EXPR_PAREN:      NodeKind = 40;
pub val NK_EXPR_ARRAY_LIT:  NodeKind = 41;
pub val NK_EXPR_STRUCT_LIT: NodeKind = 42;

# 43-47: type expressions
pub val NK_TYPE_NAME:  NodeKind = 43; pub val NK_TYPE_PTR: NodeKind = 44;
pub val NK_TYPE_REF:   NodeKind = 45; pub val NK_TYPE_ARRAY: NodeKind = 46;
pub val NK_TYPE_FUN:   NodeKind = 47;

# 48-51: misc
pub val NK_PARAM:          NodeKind = 48;
pub val NK_FIELD:          NodeKind = 49;
pub val NK_GENERIC_PARAMS: NodeKind = 50;
pub val NK_FIELD_INIT:     NodeKind = 51;

pub rec NodeSpan { start: u32; len: u32; }
pub rec Node { kind: NodeKind; tok_idx: u32; end_tok: u32; payload: u32; }

pub rec NodeStore {
    alloc:      &allocator.Allocator;
    nodes:      *Node;      node_count: u32; node_cap: u32;
    extra:      *u32;       extra_len:  u32; extra_cap: u32;
    strings:    *str;       str_count:  u32; str_cap:  u32;
}

pub fun init(alloc: &allocator.Allocator) NodeStore;
pub fun dnit(store: *NodeStore);
pub fun add_node(store: *NodeStore, kind: NodeKind, tok_idx: u32,
                 end_tok: u32, payload: u32) NodeId;
pub fun add_extra(store: *NodeStore, value: u32) u32;
pub fun add_extra_slice(store: *NodeStore, values: *u32, count: u32) u32;
pub fun intern(store: *NodeStore, text: str, len: u32) StrId;

pub fun get(store: &NodeStore, id: NodeId) &Node;
pub fun get_extra(store: &NodeStore, idx: u32) u32;
pub fun get_str(store: &NodeStore, id: StrId) str;

# kind-specific accessors
pub fun fun_name(store: &NodeStore, id: NodeId) StrId;
pub fun fun_params(store: &NodeStore, id: NodeId) NodeSpan;
pub fun fun_ret_type(store: &NodeStore, id: NodeId) NodeId;
pub fun fun_body(store: &NodeStore, id: NodeId) NodeId;
pub fun fun_generics(store: &NodeStore, id: NodeId) NodeId;
pub fun type_decl_name(store: &NodeStore, id: NodeId) StrId;
pub fun type_decl_fields(store: &NodeStore, id: NodeId) NodeSpan;
pub fun type_decl_generics(store: &NodeStore, id: NodeId) NodeId;
pub fun var_name(store: &NodeStore, id: NodeId) StrId;
pub fun var_type_node(store: &NodeStore, id: NodeId) NodeId;
pub fun var_init(store: &NodeStore, id: NodeId) NodeId;
pub fun binary_lhs(store: &NodeStore, id: NodeId) NodeId;
pub fun binary_rhs(store: &NodeStore, id: NodeId) NodeId;
pub fun binary_op(store: &NodeStore, id: NodeId) token.Tag;
pub fun call_callee(store: &NodeStore, id: NodeId) NodeId;
pub fun call_args(store: &NodeStore, id: NodeId) NodeSpan;
pub fun call_type_args(store: &NodeStore, id: NodeId) NodeId;
pub fun cond_expr(store: &NodeStore, id: NodeId) NodeId;
pub fun cond_body(store: &NodeStore, id: NodeId) NodeId;
pub fun cond_or_chain(store: &NodeStore, id: NodeId) NodeId;
pub fun block_stmts(store: &NodeStore, id: NodeId) NodeSpan;

pub fun is_decl(kind: NodeKind) bool;
pub fun is_stmt(kind: NodeKind) bool;
pub fun is_expr(kind: NodeKind) bool;
pub fun kind_name(kind: NodeKind) str;
```

---

### `compiler/parse/parser.mach` — Parser

Recursive descent with Pratt expression parsing. Operates on random-access `TokenStream`. Error recovery: on error, sets `panicking` flag, skips to next synchronization point (`;`, `}`, or top-level keyword), suppresses cascading errors until resync.

```
pub rec ParseError { tok_idx: u32; message: str; }

pub rec ParseResult {
    store:   ast.NodeStore;
    root:    ast.NodeId;
    errors:  *ParseError;
    err_len: u32;
    ok:      bool;
}

pub fun parse(stream: *lexer.TokenStream, alloc: &allocator.Allocator) ParseResult;
```

Internal productions: `parse_program`, `parse_top_level_decl`, `parse_use_decl`, `parse_pub_decl`, `parse_ext_decl`, `parse_def_decl`, `parse_fun_decl`, `parse_rec_decl`, `parse_uni_decl`, `parse_val_decl`, `parse_var_decl`, `parse_test_decl`, `parse_param_list`, `parse_field_list`, `parse_generic_params`, `parse_stmt`, `parse_block`, `parse_if_stmt`, `parse_for_stmt`, `parse_ret_stmt`, `parse_fin_stmt`, `parse_asm_stmt`, `parse_comptime_if`, `parse_expression` (Pratt), `parse_prefix`, `parse_infix`, `parse_primary`, `parse_type`, `synchronize`, `error`.

---

### `compiler/sema/type.mach` — Type System

Types stored in flat `TypeTable` indexed by `TypeId`. Primitives occupy known IDs (0-13) so they can be referenced without a table lookup in hot paths. `TK_ERROR` (255) is a sentinel that propagates silently — any expression involving an error type produces an error type without further diagnostics.

```
pub def TypeId: u32;
pub val TYPE_NIL: TypeId = 0xFFFFFFFF;

pub def TypeKind: u8;
pub val TK_VOID:       TypeKind = 0;
pub val TK_U8:         TypeKind = 1;  pub val TK_U16: TypeKind = 2;
pub val TK_U32:        TypeKind = 3;  pub val TK_U64: TypeKind = 4;
pub val TK_I8:         TypeKind = 5;  pub val TK_I16: TypeKind = 6;
pub val TK_I32:        TypeKind = 7;  pub val TK_I64: TypeKind = 8;
pub val TK_F32:        TypeKind = 9;  pub val TK_F64: TypeKind = 10;
pub val TK_BOOL:       TypeKind = 11;
pub val TK_OPAQUE_PTR: TypeKind = 12; # `ptr` — untyped, target-width
pub val TK_NIL:        TypeKind = 13;
pub val TK_POINTER:    TypeKind = 14; # `*T` or `&T` — typed pointer
pub val TK_ARRAY:      TypeKind = 15;
pub val TK_RECORD:     TypeKind = 16;
pub val TK_UNION:      TypeKind = 17;
pub val TK_FUN:        TypeKind = 18;
pub val TK_GENERIC_PARAM: TypeKind = 19;
pub val TK_ALIAS:      TypeKind = 20; # `def Name: T;` — preserves name for diagnostics
pub val TK_ERROR:      TypeKind = 255;

pub rec TypeField { name: str; type_id: TypeId; offset: u32; }

pub rec Type {
    kind:       TypeKind;
    name:       str;
    size:       u32;
    align:      u32;
    pointee:    TypeId;     # TK_POINTER: target type
    is_mutable: bool;       # TK_POINTER: true = *T, false = &T
    elem:       TypeId;     # TK_ARRAY: element type
    array_len:  u32;        # TK_ARRAY: element count
    underlying: TypeId;     # TK_ALIAS: underlying type
    fields:     *TypeField; field_count: u16;
    params:     *TypeId;    param_count: u16;
    ret_type:   TypeId;
    is_variadic: bool;
    generic_params: *str;
    generic_args:   *TypeId;
    gp_count:   u16;
    base_type:  TypeId;
    decl_sym:   scope.SymbolId;
}

pub rec TypeTable {
    alloc: &allocator.Allocator;
    types: *Type; count: u32; cap: u32;
    ptr_size: u32; ptr_align: u32;
}

pub fun init(alloc: &allocator.Allocator, ptr_size: u32, ptr_align: u32) TypeTable;
pub fun dnit(table: *TypeTable);
pub fun get(table: &TypeTable, id: TypeId) &Type;
pub fun add(table: *TypeTable, kind: TypeKind) TypeId;
pub fun primitive(table: &TypeTable, kind: TypeKind) TypeId;
pub fun primitive_by_name(table: &TypeTable, name: str) TypeId;

pub fun make_pointer(table: *TypeTable, pointee: TypeId, is_mutable: bool) TypeId;
pub fun make_array(table: *TypeTable, elem: TypeId, len: u32) TypeId;
pub fun make_record(table: *TypeTable, name: str, fields: *TypeField, count: u16) TypeId;
pub fun make_union(table: *TypeTable, name: str, fields: *TypeField, count: u16) TypeId;
pub fun make_function(table: *TypeTable, params: *TypeId, count: u16, ret: TypeId, variadic: bool) TypeId;
pub fun make_alias(table: *TypeTable, name: str, target: TypeId) TypeId;

pub fun canonical(table: &TypeTable, id: TypeId) TypeId;

pub fun is_integer(table: &TypeTable, id: TypeId) bool;
pub fun is_float(table: &TypeTable, id: TypeId) bool;
pub fun is_numeric(table: &TypeTable, id: TypeId) bool;
pub fun is_pointer(table: &TypeTable, id: TypeId) bool;
pub fun is_record(table: &TypeTable, id: TypeId) bool;
pub fun is_aggregate(table: &TypeTable, id: TypeId) bool;
pub fun is_error(table: &TypeTable, id: TypeId) bool;
pub fun types_equal(table: &TypeTable, a: TypeId, b: TypeId) bool;
pub fun can_assign(table: &TypeTable, from: TypeId, to: TypeId) bool;
pub fun can_cast(table: &TypeTable, from: TypeId, to: TypeId) bool;

pub fun compute_record_layout(table: *TypeTable, id: TypeId);
pub fun compute_union_layout(table: *TypeTable, id: TypeId);
pub fun get_field(table: &TypeTable, id: TypeId, name: str) *TypeField;
pub fun display_name(table: &TypeTable, id: TypeId) str;
```

---

### `compiler/sema/scope.mach` — Symbols and Scopes

```
pub def SymbolId: u32;  pub val SYM_NIL: SymbolId = 0xFFFFFFFF;
pub def ScopeId: u32;   pub val SCOPE_NIL: ScopeId = 0xFFFFFFFF;

pub def SymbolKind: u8;
pub val SK_VAL:    SymbolKind = 0;  pub val SK_VAR:   SymbolKind = 1;
pub val SK_FUN:    SymbolKind = 2;  pub val SK_TYPE:  SymbolKind = 3;
pub val SK_FIELD:  SymbolKind = 4;  pub val SK_PARAM: SymbolKind = 5;
pub val SK_MODULE: SymbolKind = 6;  pub val SK_EXT:   SymbolKind = 7;

pub rec Symbol {
    name:         str;          # interned in session StringPool
    kind:         SymbolKind;
    decl_node:    ast.NodeId;
    type_id:      type.TypeId;
    scope:        ScopeId;
    module:       module.ModuleId;
    is_public:    bool;
    is_mutable:   bool;
    is_generic:   bool;
    is_resolving: bool;
    generic_node: ast.NodeId;
    export_name:  str;
}

pub rec Scope {
    parent:   ScopeId;
    name_map: map.Map[str, SymbolId];
}

pub rec SymbolTable {
    alloc:       &allocator.Allocator;
    symbols:     *Symbol;  sym_count: u32;   sym_cap: u32;
    scopes:      *Scope;   scope_count: u32; scope_cap: u32;
    current:     ScopeId;
}

pub fun init(alloc: &allocator.Allocator) SymbolTable;
pub fun dnit(table: *SymbolTable);
pub fun push_scope(table: *SymbolTable) ScopeId;
pub fun pop_scope(table: *SymbolTable);
pub fun define(table: *SymbolTable, name: str, kind: SymbolKind) SymbolId;
pub fun define_in(table: *SymbolTable, scope: ScopeId, name: str, kind: SymbolKind) SymbolId;
pub fun lookup(table: &SymbolTable, name: str) SymbolId;
pub fun lookup_local(table: &SymbolTable, scope: ScopeId, name: str) SymbolId;
pub fun get_sym(table: &SymbolTable, id: SymbolId) &Symbol;
pub fun get_sym_mut(table: *SymbolTable, id: SymbolId) *Symbol;
pub fun current_scope(table: &SymbolTable) ScopeId;
```

---

### `compiler/sema/module.mach` — Module Graph

Modules are loaded, lexed, and parsed independently (parallelizable). The dependency graph is built by scanning `use` declarations. `$if` target predicates at module level are evaluated during `load_graph` via a lightweight comptime evaluator (target properties only — no `$size_of` or type-dependent intrinsics at this stage). Cycles are detected during graph construction via a `Set[str]` of in-flight module paths.

Side tables (`ExprTypeMap`, `NodeSymMap`) live on the `Module` record alongside the `NodeStore` they annotate.

```
pub def ModuleId: u32;  pub val MOD_NIL: ModuleId = 0xFFFFFFFF;

pub def Phase: u8;
pub val PHASE_LOADED:    Phase = 0;
pub val PHASE_COLLECTED: Phase = 1;
pub val PHASE_RESOLVED:  Phase = 2;
pub val PHASE_CHECKED:   Phase = 3;

pub rec Module {
    path:         str;
    file_path:    str;
    source:       str;
    tokens:       lexer.TokenStream;
    store:        ast.NodeStore;
    root:         ast.NodeId;
    scope:        scope.ScopeId;
    phase:        Phase;
    aliases:      map.Map[str, ModuleId];
    imports:      *ModuleId;  import_count: u32;  import_cap: u32;
    expr_types:   check.ExprTypeMap;
    node_syms:    check.NodeSymMap;
    content_hash: u64;
}

pub rec ModuleTable {
    alloc:     &allocator.Allocator;
    modules:   *Module;  count: u32; cap: u32;
    path_map:  map.Map[str, ModuleId];
    order:     *ModuleId; order_len: u32;
}

pub fun init(alloc: &allocator.Allocator) ModuleTable;
pub fun dnit(table: *ModuleTable);
pub fun create(table: *ModuleTable, path: str, file_path: str, source: str) ModuleId;
pub fun find(table: &ModuleTable, path: str) ModuleId;
pub fun get(table: &ModuleTable, id: ModuleId) &Module;
pub fun get_mut(table: *ModuleTable, id: ModuleId) *Module;
pub fun add_alias(table: *ModuleTable, on: ModuleId, alias: str, target: ModuleId);
pub fun add_import(table: *ModuleTable, on: ModuleId, target: ModuleId);
pub fun load_graph(table: *ModuleTable, root: ModuleId, src_root: str,
                   dep_root: str, project_id: str);
pub fun topo_sort(table: *ModuleTable) bool;
pub fun order_count(table: &ModuleTable) u32;
pub fun order_at(table: &ModuleTable, idx: u32) ModuleId;
pub fun resolve_alias(table: &ModuleTable, on: ModuleId, alias: str) ModuleId;
pub fun invalidate(table: *ModuleTable, id: ModuleId, to_phase: Phase);
```

---

### `compiler/sema/sema.mach` — Semantic Analyzer

```
pub rec Sema {
    alloc:   &allocator.Allocator;
    modules: *module.ModuleTable;
    symbols: *scope.SymbolTable;
    types:   *type.TypeTable;
    mono:    *mono.MonoCache;
    diag:    *diag.DiagList;
    strings: *session.StringPool;
}

pub fun init(alloc: &allocator.Allocator, modules: *module.ModuleTable,
             diag: *diag.DiagList, strings: *session.StringPool, ptr_size: u32) Sema;
pub fun dnit(sema: *Sema);
pub fun analyze(sema: *Sema);
pub fun phase_collect(sema: *Sema);
pub fun phase_resolve(sema: *Sema);
pub fun phase_check_bodies(sema: *Sema);
pub fun phase_validate(sema: *Sema);
```

---

### `compiler/sema/resolve.mach` — Name and Type Resolution

```
pub fun collect_all(sema: *sema.Sema);
pub fun collect_module(sema: *sema.Sema, mod_id: module.ModuleId);
pub fun resolve_all(sema: *sema.Sema);
pub fun resolve_module(sema: *sema.Sema, mod_id: module.ModuleId);
pub fun resolve_type_expr(sema: *sema.Sema, mod_id: module.ModuleId,
                          node_id: ast.NodeId) type.TypeId;
pub fun lookup_symbol(sema: *sema.Sema, mod_id: module.ModuleId, name: str) scope.SymbolId;
```

---

### `compiler/sema/check.mach` — Body Resolution and Validation

Side tables for sema annotations. Parallel arrays indexed by `NodeId`. Owned by `Module`, not `Sema`.

Expression resolution takes an `expected` TypeId for literal coercion. Pass `TYPE_NIL` when no expectation exists. Integer/float literals check whether the value fits the expected type's range and coerce if so.

```
pub rec ExprTypeMap { types: *type.TypeId; count: u32; }
pub rec NodeSymMap  { syms: *scope.SymbolId; count: u32; }
pub rec FnContext   { fn_sym: scope.SymbolId; ret_type: type.TypeId; loop_depth: u32; }

pub fun resolve_bodies(sema: *sema.Sema);
pub fun resolve_instantiation_bodies(sema: *sema.Sema);
pub fun check_all(sema: *sema.Sema);
pub fun init_expr_types(alloc: &allocator.Allocator, node_count: u32) ExprTypeMap;
pub fun init_node_syms(alloc: &allocator.Allocator, node_count: u32) NodeSymMap;
pub fun set_type(etm: *ExprTypeMap, node_id: ast.NodeId, type_id: type.TypeId);
pub fun get_type(etm: &ExprTypeMap, node_id: ast.NodeId) type.TypeId;
pub fun set_sym(nsm: *NodeSymMap, node_id: ast.NodeId, sym_id: scope.SymbolId);
pub fun get_sym(nsm: &NodeSymMap, node_id: ast.NodeId) scope.SymbolId;
```

Key internal signature:
```
fun resolve_expr(sema: *sema.Sema, mod_id: module.ModuleId,
                 ctx: *FnContext, node_id: ast.NodeId,
                 expected: type.TypeId) type.TypeId;
```

---

### `compiler/sema/mono.mach` — Monomorphization

Isolated cache. Each instantiation gets its own cloned `NodeStore`. Cycle detection via `Set[str]` of in-flight mangled names + hard depth limit.

```
pub rec FnInst {
    mangled_name:  str;
    func_node:     ast.NodeId;
    func_store:    ast.NodeStore;
    func_type:     type.TypeId;
    param_names:   *str;
    param_types:   *type.TypeId;
    param_count:   u32;
    origin_module: module.ModuleId;
    body_resolved: bool;
}

pub rec MonoCache {
    alloc:     &allocator.Allocator;
    type_map:  map.Map[str, type.TypeId];
    fn_map:    map.Map[str, u32];
    fn_insts:  *FnInst;  fn_count: u32; fn_cap: u32;
    in_flight: set.Set[str];
    depth:     u32;
}

pub fun init(alloc: &allocator.Allocator) MonoCache;
pub fun dnit(cache: *MonoCache);
pub fun instantiate_fn(cache: *MonoCache, sema: *sema.Sema, mod_id: module.ModuleId,
                       fn_sym: scope.SymbolId, type_args: ast.NodeId) type.TypeId;
pub fun instantiate_type(cache: *MonoCache, sema: *sema.Sema, mod_id: module.ModuleId,
                         type_sym: scope.SymbolId, args: *type.TypeId, arg_count: u32) type.TypeId;
pub fun fn_count(cache: &MonoCache) u32;
pub fun get_fn(cache: &MonoCache, idx: u32) &FnInst;
pub fun mangle_name(cache: *MonoCache, base: str, args: *type.TypeId, count: u32,
                    types: &type.TypeTable) str;
```

---

### `compiler/sema/comptime.mach` — Compile-Time Evaluation

Two tiers:
- **Loader-level** (`eval_target_condition`): evaluates `$if` predicates during module loading. Only has access to target properties (os, isa, abi). No type or symbol information yet.
- **Sema-level** (`eval`, `eval_size_of`, etc.): full evaluation with access to the type system. Used during phase 3 body resolution.

```
pub def CKind: u8;
pub val CK_INT: CKind = 1;    pub val CK_BOOL: CKind = 2;
pub val CK_STRING: CKind = 3; pub val CK_NIL: CKind = 4;

pub rec CValue { kind: CKind; int_val: i64; bool_val: bool; str_val: str; }

pub fun eval_target_condition(node_store: &ast.NodeStore, node_id: ast.NodeId,
                              target_os: str, target_isa: str) Result[bool, str];

pub fun eval(sema: *sema.Sema, mod_id: module.ModuleId,
             node_id: ast.NodeId) Result[CValue, str];
pub fun eval_condition(sema: *sema.Sema, mod_id: module.ModuleId,
                       node_id: ast.NodeId) Result[bool, str];
pub fun eval_size_of(sema: *sema.Sema, type_id: type.TypeId) i64;
pub fun eval_align_of(sema: *sema.Sema, type_id: type.TypeId) i64;
pub fun eval_offset_of(sema: *sema.Sema, type_id: type.TypeId, field: str) Result[i64, str];
```

---

### `compiler/lower/lower.mach` — IR Lowering

```
pub rec LocalVar { name: str; type_id: type.TypeId; offset: i64; size: usize; align: usize; }
pub rec LoopState { brk_label: str; cnt_label: str; }
pub rec DeferState { nodes: *ast.NodeId; count: i32; cap: i32; mark: i32; }
pub rec ReturnState { ret_type: type.TypeId; has_sret: bool; sret_vreg: u16; }

pub rec LowerContext {
    alloc: &allocator.Allocator;
    target: *target.Target;
    builder: *masm.module.ModuleBuilder;
    sema: *sema.Sema;
    func: *masm.function.Function;
    block: *masm.function.Block;
    vreg_next: i32; label_counter: i32; str_counter: i32;
    locals: *LocalVar; local_count: i32; local_cap: i32;
    stack_offset: i64;
    rs: ReturnState; ls: LoopState; ds: DeferState;
    current_file: u16; current_line: i32;
    ptr_size: u8;
}

pub fun init(alloc: &allocator.Allocator, target: *target.Target,
             builder: *masm.module.ModuleBuilder, sema: *sema.Sema) Result[LowerContext, str];
pub fun dnit(ctx: *LowerContext);
pub fun lower_module(ctx: *LowerContext, mod_id: module.ModuleId);
pub fun lower_function(ctx: *LowerContext, node_id: ast.NodeId);
```

### `compiler/lower/expr.mach`
```
pub rec LvalueResult { base_vreg: u16; disp_off: u16; size: u8; }
pub fun lower_expr(ctx: *lower.LowerContext, node_id: ast.NodeId) u16;
pub fun lower_lvalue(ctx: *lower.LowerContext, node_id: ast.NodeId) LvalueResult;
```

### `compiler/lower/stmt.mach`
```
pub fun lower_stmt(ctx: *lower.LowerContext, node_id: ast.NodeId);
```

### `compiler/lower/data.mach`
```
pub fun lower_global_var(ctx: *lower.LowerContext, node_id: ast.NodeId);
pub fun lower_string_constant(ctx: *lower.LowerContext, value: str) str;
```

---

### `compiler/opt/pass.mach` — Optimization Pass Interface

Passes operate on `masm.Function` in-place. `PassPipeline` is the primary type.

```
pub def PassFn: fun(&allocator.Allocator, *masm.function.Function) bool;

pub rec PassDesc { name: str; run: PassFn; }

pub rec PassPipeline {
    alloc:  &allocator.Allocator;
    passes: *PassDesc;
    count:  u32;
    cap:    u32;
}

pub fun init(alloc: &allocator.Allocator) PassPipeline;
pub fun add(p: *PassPipeline, name: str, run: PassFn);
pub fun run(p: &PassPipeline, func: *masm.function.Function) bool;
pub fun dnit(p: *PassPipeline);

pub fun default_pipeline(alloc: &allocator.Allocator, opt_level: u8) PassPipeline;
```

### `compiler/opt/dce.mach`
```
pub fun run(alloc: &allocator.Allocator, func: *masm.function.Function) bool;
```

### `compiler/opt/constfold.mach`
```
pub fun run(alloc: &allocator.Allocator, func: *masm.function.Function) bool;
```

---

### `compiler/codegen.mach` — Per-Function Codegen

Orchestrates the per-function backend pipeline. Each function is independent — this is the parallelism boundary.

```
pub rec CodegenResult {
    ok:          bool;
    spill_size:  usize;
    callee_mask: u32;
    local_size:  usize;
}

pub fun compile_function(func: *masm.function.Function,
                         local_size: usize,
                         target: *target.Target,
                         opt: *pass.PassPipeline,
                         alloc: &allocator.Allocator,
                         obj: *of.ObjWriter) CodegenResult;
```

Pipeline within `compile_function`:
1. `masm.verify.verify_function(func)` — validate IR
2. `pass.run(pipeline, func)` — optimization passes
3. `masm.cfg.build_cfg(alloc, func)` — build CFG
4. `masm.cfg.compute_liveness(alloc, &cfg, func)` — backward dataflow liveness
5. ISel: `target.isa.isel(...)` per instruction → `InstBuf`
6. Peephole: `target.isa.peephole(...)`
7. RegAlloc: `regalloc.alloc_function(...)` using CFG + liveness
8. Frame layout + operand fixup (stack offsets)
9. Prologue/epilogue: `target.isa.prologue(...)` / `target.isa.epilogue(...)`
10. Encode: `target.isa.encode(...)` per isa.Inst → section bytes

---

### `compiler/regalloc.mach` — Register Allocation

Target-independent linear scan. Uses `masm.cfg` liveness (backward dataflow) — mandatory, not optional.

```
pub rec LiveRange {
    vreg:      i32;
    start:     i32;
    end_pos:   i32;
    use_count: i32;
    size:      u8;
    reg_class: u8;
    preg:      i32;
    spill_off: i64;
    spilled:   bool;
}

pub rec RegAllocResult {
    out:         isa.InstBuf;
    spill_size:  usize;
    callee_mask: u32;
    ok:          bool;
}

pub fun alloc_function(alloc: &allocator.Allocator,
                       i: *isa.ISA, a: *abi.ABI,
                       insts: *isa.InstBuf,
                       local_size: usize,
                       cfg: *masm.cfg.CFG,
                       live: *masm.cfg.LiveSets) RegAllocResult;
```

---

### `compiler/linker.mach` — Static Linker

Format-agnostic. Calls `isa.resolve_reloc()` for relocation resolution and `of.ExecFormat.emit()` for output.

```
pub rec LinkResult { output_path: str; ok: bool; error: str; }

pub fun link(alloc: &allocator.Allocator, obj_paths: *str, obj_count: i32,
             output_path: str, target: *target.Target) LinkResult;
```

---

### `compiler/target/target.mach` — Target Composition

```
pub rec Target {
    isa:      isa.ISA;
    abi:      abi.ABI;
    os:       OS;
    of:       of.ObjectFormat;
    ptr_size: usize;
}

pub fun select(arch: str, os_name: str, abi_name: str, fmt: str,
               alloc: &allocator.Allocator) Result[Target, str];
pub fun select_native(alloc: &allocator.Allocator) Result[Target, str];
pub fun dnit(target: *Target);
```

---

### `compiler/target/os.mach` — OS Configuration

```
pub rec OS {
    name:         str;     # "linux", "darwin", "windows"
    entry_symbol: str;     # "_start", "main", "_main"
    base_addr:    u64;     # default executable load address
    page_size:    usize;   # memory page size
}

pub fun from_name(name: str) Result[OS, str];
pub fun linux() OS;
pub fun darwin() OS;
pub fun windows() OS;
```

---

### `compiler/target/abi.mach` — ABI Interface

```
pub def ParamClass: u8;
pub val CLASS_GP:    ParamClass = 0;
pub val CLASS_FP:    ParamClass = 1;
pub val CLASS_STACK: ParamClass = 2;
pub val CLASS_SRET:  ParamClass = 3;
pub val CLASS_BYREF: ParamClass = 4;

pub rec ParamSlot { class: ParamClass; reg: i32; offset: i64; size: usize; }

pub rec ABI {
    ctx:            ptr;
    name:           str;
    classify_param: fun(ptr, i32, usize, usize, bool, bool, i32, i32) ParamSlot;
    classify_return: fun(ptr, usize, usize, bool, bool) ParamSlot;
    gp_param_regs:  fun(ptr) isa.RegisterFile;
    fp_param_regs:  fun(ptr) isa.RegisterFile;
    callee_saved:   fun(ptr) isa.RegisterFile;
    stack_align:    fun(ptr) usize;
    red_zone:       fun(ptr) usize;
}
```

---

### `compiler/target/isa.mach` — ISA Interface

```
pub rec Register { id: i32; name: str; class: u8; width: u8; }
pub rec RegisterFile { regs: *Register; count: i32; }

pub rec Operand {
    kind: u8; size: u8; reg: i32; imm: i64;
    base: i32; index: i32; scale: i32; disp: i64;
    sym_id: u64; label: str;
}

pub rec Inst {
    opcode: i32;
    dst: Operand; src1: Operand; src2: Operand;
    size: u8; flags: u32;
    src_line: i32; src_file: u16;
}

pub rec InstBuf {
    insts: *Inst; count: i32; cap: i32;
    alloc: &allocator.Allocator;
}

pub rec ISA {
    ctx:           ptr;
    name:          str;
    ptr_size:      usize;
    gp_regs:       fun(ptr) RegisterFile;
    fp_regs:       fun(ptr) RegisterFile;
    isel:          fun(ptr, *masm.inst.Instruction, *masm.function.Function, *InstBuf) bool;
    encode:        fun(ptr, *Inst, *of.Section, &allocator.Allocator) i32;
    peephole:      fun(ptr, *InstBuf);
    frame_reg:     fun(ptr) i32;
    reserved_regs: fun(ptr) RegisterFile;
    emit_spill:    fun(ptr, *InstBuf, i32, i64, u8, i32) i32;
    emit_reload:   fun(ptr, *InstBuf, i32, i64, u8, i32) i32;
    prologue:      fun(ptr, usize, u32, *InstBuf) i32;
    epilogue:      fun(ptr, u32, *InstBuf) i32;
    resolve_reloc: fun(ptr, u8, u64, u64, i64, *u8) i32;
}

pub fun buf_init(alloc: &allocator.Allocator) InstBuf;
pub fun buf_append(buf: *InstBuf, inst: Inst) bool;
pub fun buf_dnit(buf: *InstBuf);
pub fun make_reg(reg: i32, size: u8) Operand;
pub fun make_imm(imm: i64, size: u8) Operand;
pub fun make_mem(base: i32, index: i32, scale: i32, disp: i64, size: u8) Operand;
pub fun make_label(label: str) Operand;
```

Adding a new ISA (e.g., aarch64): implement the `ISA` function pointer record, create `isa/aarch64/defs.mach`, `isel.mach`, `encode.mach`. No other files change.

---

### `compiler/target/of.mach` — Object Format Interface

```
pub rec Section { name: str; kind: u8; flags: u32; data: *u8; size: usize; align: usize; }

pub rec ObjWriter {
    ctx: ptr; name: str;
    add_section:    fun(ptr, str, u8, u32, usize) u64;
    write:          fun(ptr, u64, *u8, usize);
    add_symbol:     fun(ptr, str, u8, u8, u64, usize, usize) u64;
    add_reloc:      fun(ptr, u64, usize, u64, u32, i64);
    section_offset: fun(ptr, u64) usize;
    finalize:       fun(ptr, *u8, *usize);
    dnit:           fun(ptr);
}

pub rec ExecFormat {
    ctx: ptr;
    header_size: fun(ptr, i32) usize;
    emit:        fun(ptr, *Section, i32, u64, u64, *u8, *usize);
    dnit:        fun(ptr);
}

pub rec ObjectFormat { name: str; obj: ObjWriter; exec: ExecFormat; }
```

---

## Extension Points

| Feature | Integration point | Scope |
|---------|-------------------|-------|
| New ISA (aarch64, riscv) | `target/isa/<name>/` — implement ISA interface | Self-contained directory |
| New ABI (win64, aapcs) | `target/abi/<name>.mach` — implement ABI interface | Single file |
| New object format (Mach-O, COFF) | `target/of/<name>.mach` — implement ObjWriter + ExecFormat | Single file |
| Optimization pass | `opt/<name>.mach` — implement `PassFn`, register in pipeline | Single file |
| Language feature (traits, closures) | `parse/` for syntax, `sema/` for checking, `lower/` for codegen | Across frontend |
| Incremental compilation | `module.mach` content_hash + `invalidate()` propagation | Module table |
| DWARF debug info | `target/of.mach` DebugInfo interface + `codegen.mach` emission | Backend |
| LSP integration | `session.mach` query APIs after `analyze()` | Public API |
| JSON diagnostics | `diag.mach` `print_json()` formatter | Diagnostics |
| Cross-compilation | `target.mach` `select()` with explicit arch/os/abi/fmt | Target |
| Parallel codegen | `codegen.mach` called per-function from thread pool | Codegen boundary |

## Verification

- `cmach build .` at each implementation stage
- `masm verify` on all generated IR after lowering
- Existing `src/lang_test/` test suite for language feature coverage
- 4-stage bootstrap: cmach → imach → smach → mach
