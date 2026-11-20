[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **interpretation**

# mach.front.ast.interpretation

how name resolution reads a parse that the syntax alone cannot settle

the parser keeps both readings of `name[` and reads a positional literal as written. name
resolution records one `Choice` per expression, a bracket read as generic arguments or as an
index and a literal read as a tag case, and one `Head` per type node, a literal head read as a
type and its case. `get` and `type_node` answer a node as resolution read it, and the parsed
tree is never rewritten.

Source: [`src/front/ast/interpretation.mach`](../../../../src/front/ast/interpretation.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.expr`](expr.md), [`mach.front.ast.id`](id.md), [`mach.front.ast.type`](type.md).

| Type | | |
| :--- | :--- | :--- |
| [`Choice`](#choice) | `def` |  |
| [`Head`](#head) | `def` | how name resolution read a literal head's path: as written, or as a type and the tag case its last segment selects |

| On `Choice` | |
| :--- | :--- |
| [`PARSED`](#choice), [`GENERIC`](#choice), [`INDEX_CALL`](#choice), [`INDEX_VALUE`](#choice), [`REJECTED`](#choice) | |
| [`CASE_LITERAL`](#choice) | a positional literal whose head names a tag case: a case construction carrying its elements as the payload, not a vector's lanes |
| [`get`](#get) | the expression `eid` as name resolution read it, a copy rewritten by its choice |

| On `Head` | |
| :--- | :--- |
| [`HEAD_PARSED`](#head), [`HEAD_CASE`](#head) | |
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
pub fun get(a: *mach.front.ast.Ast, choices: *Choice, count: usize, eid: mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.base.fail.Fail]
```

the expression `eid` as name resolution read it, a copy rewritten by its choice

| Parameter | |
| :--- | :--- |
| `a` | the parsed tree |
| `choices` | one choice per expression of the tree |
| `count` | how many choices, which must equal the tree's expression count |
| `eid` | the expression |
| **returns** | the expression as read, or a failure when the choices do not match the tree, the id is out of range or the choice does not fit the node |

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
pub fun type_node(a: *mach.front.ast.Ast, source: str, heads: *Head, count: usize, tid: mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.base.fail.Fail]
```

the type node `tid` as name resolution read it: a split head names the type by its path's
prefix and the case by its last segment

## Functions

### split

```mach
pub fun split(node: mach.front.ast.type.Type, source: str) res[mach.front.ast.type.Type, mach.base.fail.Fail]
```

a named head read as type and case: the path's prefix names the type, its last segment the case

