[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **interpretation**

# mach.front.ast.interpretation

how name resolution reads a parse that the syntax alone cannot settle

the parser keeps both readings of `name[` and reads a positional literal as written. name
resolution records one `Choice` per expression, a bracket read as generic arguments or as an
index and a literal read as a tag case, and one `Head` per type node, a literal head read as a
type and its case. `get` and `type_node` answer a node as resolution read it, and the parsed
tree is never rewritten.

<sub>source: [`src/front/ast/interpretation.mach`](../../../../src/front/ast/interpretation.mach) · imported by 2 modules</sub>

<sub>`ast` is [`mach.front.ast`](../ast.md) · `ast_id` is [`mach.front.ast.id`](id.md) · `ast_expr` is [`mach.front.ast.expr`](expr.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `ast_type` is [`mach.front.ast.type`](type.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Choice`](#choice) | `def` |  |
| [`Head`](#head) | `def` | how name resolution read a literal head's path: as written, or as a type and the tag case its last segment selects |

| On `Choice` | |
| :--- | :--- |
| [`PARSED`](#choice) · [`GENERIC`](#choice) · [`INDEX_CALL`](#choice) · [`INDEX_VALUE`](#choice) · [`REJECTED`](#choice) | |
| [`CASE_LITERAL`](#choice) | a positional literal whose head names a tag case: a case construction carrying its elements as the payload, not a vector's lanes |
| [`get`](#get) | |

| On `Head` | |
| :--- | :--- |
| [`HEAD_PARSED`](#head) · [`HEAD_CASE`](#head) | |
| [`type_node`](#type_node) | the type node `tid` as name resolution read it: a split head names the type by its path's prefix and the case by its last segment |

| Functions | |
| :--- | :--- |
| [`split`](#split) | a named head read as type and case: the path's prefix names the type, its last segment the case |

---

## Choice

```mach
pub def Choice: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `PARSED` | `0` |  |
| `GENERIC` | `1` |  |
| `INDEX_CALL` | `2` |  |
| `INDEX_VALUE` | `3` |  |
| `REJECTED` | `4` |  |
| `CASE_LITERAL` | `5` | a positional literal whose head names a tag case: a case construction carrying its elements as the payload, not a vector's lanes |

### get

```mach
pub fun get(a: *ast.Ast, choices: *Choice, count: usize, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail]
```

## Head

```mach
pub def Head: u8
```

how name resolution read a literal head's path: as written, or as a type and the tag case its
last segment selects

| Name | Value | |
| :--- | :--- | :--- |
| `HEAD_PARSED` | `0` |  |
| `HEAD_CASE` | `1` |  |

### type_node

```mach
pub fun type_node(a: *ast.Ast, source: str, heads: *Head, count: usize, tid: ast_id.TypeId) res[ast_type.Type, fail.Fail]
```

the type node `tid` as name resolution read it: a split head names the type by its path's
prefix and the case by its last segment

## Functions

### split

```mach
pub fun split(node: ast_type.Type, source: str) res[ast_type.Type, fail.Fail]
```

a named head read as type and case: the path's prefix names the type, its last segment the case

