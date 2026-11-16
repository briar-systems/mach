# mach.front.ast.interpretation

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

## val CASE_LITERAL

```mach
pub val CASE_LITERAL: Choice = 5
```

a positional literal whose head names a tag case: a case construction carrying its elements
as the payload, not a vector's lanes

## def Head

```mach
pub def Head: u8
```

how name resolution read a literal head's path: as written, or as a type and the tag case its
last segment selects

## val HEAD_PARSED

```mach
pub val HEAD_PARSED: Head = 0
```

## val HEAD_CASE

```mach
pub val HEAD_CASE:   Head = 1
```

## fun get

```mach
pub fun get(a: *ast.Ast, choices: *Choice, count: usize, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun type_node

```mach
pub fun type_node(a: *ast.Ast, source: str, heads: *Head, count: usize, tid: ast_id.TypeId) res[ast_type.Type, fail.Fail];
```

the type node `tid` as name resolution read it: a split head names the type by its path's
prefix and the case by its last segment

## fun split

```mach
pub fun split(node: ast_type.Type, source: str) res[ast_type.Type, fail.Fail];
```

a named head read as type and case: the path's prefix names the type, its last segment the case

