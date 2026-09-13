# mach.lang.fe.parser.grammar

## fun parse_module

```mach
pub fun parse_module(p: *state.Parser) id.ModuleNodeId;
```

## fun parse_decl

```mach
pub fun parse_decl(p: *state.Parser) id.DeclId;
```

## fun parse_stmt

```mach
pub fun parse_stmt(p: *state.Parser) id.StmtId;
```

## fun parse_expr

```mach
pub fun parse_expr(p: *state.Parser) id.ExprId;
```

## fun parse_expr_bp

```mach
pub fun parse_expr_bp(p: *state.Parser, min_bp: u8) id.ExprId;
```

## fun parse_type

```mach
pub fun parse_type(p: *state.Parser) id.TypeId;
```

## val REMOVED_STRIP_MSG

```mach
pub val REMOVED_STRIP_MSG: str = "`:^` and `:^T` were removed in 5.0.0
```

