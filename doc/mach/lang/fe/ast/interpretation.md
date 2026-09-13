# mach.lang.fe.ast.interpretation

## def Choice

```mach
pub def Choice: u8
```

## val PARSED

```mach
pub val PARSED:      Choice = 0
```

## val GENERIC

```mach
pub val GENERIC:     Choice = 1
```

## val INDEX_CALL

```mach
pub val INDEX_CALL:  Choice = 2
```

## val INDEX_VALUE

```mach
pub val INDEX_VALUE: Choice = 3
```

## val REJECTED

```mach
pub val REJECTED:    Choice = 4
```

## fun get

```mach
pub fun get(a: *ast.Ast, choices: *Choice, count: usize, eid: id.ExprId) res[expr.Expr, fail.Fail];
```

