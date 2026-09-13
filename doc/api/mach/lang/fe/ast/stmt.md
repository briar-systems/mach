# mach.lang.fe.ast.stmt

## def StmtKind

```mach
pub def StmtKind: u8
```

## val STMT_KIND_EXPR

```mach
pub val STMT_KIND_EXPR:          StmtKind = 0
```

## val STMT_KIND_BLOCK

```mach
pub val STMT_KIND_BLOCK:         StmtKind = 1
```

## val STMT_KIND_IF

```mach
pub val STMT_KIND_IF:            StmtKind = 2
```

## val STMT_KIND_FOR

```mach
pub val STMT_KIND_FOR:           StmtKind = 3
```

## val STMT_KIND_RET

```mach
pub val STMT_KIND_RET:           StmtKind = 4
```

## val STMT_KIND_BRK

```mach
pub val STMT_KIND_BRK:           StmtKind = 5
```

## val STMT_KIND_CNT

```mach
pub val STMT_KIND_CNT:           StmtKind = 6
```

## val STMT_KIND_FIN

```mach
pub val STMT_KIND_FIN:           StmtKind = 7
```

## val STMT_KIND_DECL

```mach
pub val STMT_KIND_DECL:          StmtKind = 8
```

## val STMT_KIND_ASM

```mach
pub val STMT_KIND_ASM:           StmtKind = 9
```

## val STMT_KIND_COMPTIME_IF

```mach
pub val STMT_KIND_COMPTIME_IF:   StmtKind = 10
```

## val STMT_KIND_COMPTIME_EACH

```mach
pub val STMT_KIND_COMPTIME_EACH: StmtKind = 11
```

## val STMT_KIND_COMPTIME_ATTR

```mach
pub val STMT_KIND_COMPTIME_ATTR: StmtKind = 12
```

## val STMT_KIND_ERROR

```mach
pub val STMT_KIND_ERROR:         StmtKind = 255
```

## rec StmtExpr

```mach
pub rec StmtExpr;
```

## rec StmtBlock

```mach
pub rec StmtBlock;
```

## rec StmtIf

```mach
pub rec StmtIf;
```

## rec StmtFor

```mach
pub rec StmtFor;
```

## rec StmtRet

```mach
pub rec StmtRet;
```

## rec StmtFin

```mach
pub rec StmtFin;
```

## rec StmtDecl

```mach
pub rec StmtDecl;
```

## rec StmtAsm

```mach
pub rec StmtAsm;
```

## rec StmtComptimeIf

```mach
pub rec StmtComptimeIf;
```

## rec StmtComptimeEach

```mach
pub rec StmtComptimeEach;
```

## rec StmtComptimeAttr

```mach
pub rec StmtComptimeAttr;
```

## rec Stmt

```mach
pub rec Stmt;
```

