[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **id**

# mach.front.ast.id

the ids of the syntax nodes in one tree

each id is the index of its node in the table for its kind, with all ones as its nil. the ids
are transparent `def` aliases of `u32`, so the compiler does not keep one kind of id from
standing where another, or a bare integer, is meant.

Source: [`src/front/ast/id.mach`](../../../../src/front/ast/id.mach)

| Type | | |
| :--- | :--- | :--- |
| [`ExprId`](#exprid) | `def` | an expression node |
| [`StmtId`](#stmtid) | `def` | a statement node |
| [`DeclId`](#declid) | `def` | a declaration node |
| [`TypeId`](#typeid) | `def` | a type-syntax node, distinct from the semantic type.TypeId |
| [`ModuleNodeId`](#modulenodeid) | `def` | a module node |

| On `ExprId` | |
| :--- | :--- |
| [`EXPR_NIL`](#exprid) | |
| [`expr`](#expr) | the expression id of a table index |
| [`expr_index`](#expr_index) | the expression table index an id names |
| [`expr_same`](#expr_same) | whether two expression ids name the same node |
| [`expr_is_nil`](#expr_is_nil) | whether an expression id is the nil id |

| On `StmtId` | |
| :--- | :--- |
| [`STMT_NIL`](#stmtid) | |
| [`stmt`](#stmt) | the statement id of a table index |
| [`stmt_index`](#stmt_index) | the statement table index an id names |
| [`stmt_same`](#stmt_same) | whether two statement ids name the same node |
| [`stmt_is_nil`](#stmt_is_nil) | whether a statement id is the nil id |

| On `DeclId` | |
| :--- | :--- |
| [`DECL_NIL`](#declid) | |
| [`decl`](#decl) | the declaration id of a table index |
| [`decl_index`](#decl_index) | the declaration table index an id names |
| [`decl_same`](#decl_same) | whether two declaration ids name the same node |
| [`decl_is_nil`](#decl_is_nil) | whether a declaration id is the nil id |

| On `TypeId` | |
| :--- | :--- |
| [`TYPE_NIL`](#typeid) | |
| [`type`](#type) | the type-syntax id of a table index |
| [`type_index`](#type_index) | the type-syntax table index an id names |
| [`type_is_nil`](#type_is_nil) | whether a type-syntax id is the nil id |

| On `ModuleNodeId` | |
| :--- | :--- |
| [`MODULE_NODE_NIL`](#modulenodeid) | |
| [`module_node`](#module_node) | the module node id of a table index |
| [`module_node_index`](#module_node_index) | the module node table index an id names |
| [`module_node_is_nil`](#module_node_is_nil) | whether a module node id is the nil id |

---

## ExprId

```mach
pub def ExprId: u32
```

an expression node

| Name | Value | |
| :--- | :--- | :--- |
| `EXPR_NIL` | `0xFFFFFFFF` |  |

### expr

```mach
pub fun expr(index: u32) ExprId
```

the expression id of a table index

| Parameter | |
| :--- | :--- |
| `index` | index into the expression table |
| **returns** | the id naming that row |

### expr_index

```mach
pub fun expr_index(id: ExprId) u32
```

the expression table index an id names

| Parameter | |
| :--- | :--- |
| `id` | the expression id |
| **returns** | its table index, all ones for the nil id |

### expr_same

```mach
pub fun expr_same(left: ExprId, right: ExprId) bool
```

whether two expression ids name the same node

| Parameter | |
| :--- | :--- |
| `left` | first id |
| `right` | second id |
| **returns** | true when the ids are equal |

### expr_is_nil

```mach
pub fun expr_is_nil(id: ExprId) bool
```

whether an expression id is the nil id

| Parameter | |
| :--- | :--- |
| `id` | the expression id |
| **returns** | true when it is EXPR_NIL |

## StmtId

```mach
pub def StmtId: u32
```

a statement node

| Name | Value | |
| :--- | :--- | :--- |
| `STMT_NIL` | `0xFFFFFFFF` |  |

### stmt

```mach
pub fun stmt(index: u32) StmtId
```

the statement id of a table index

| Parameter | |
| :--- | :--- |
| `index` | index into the statement table |
| **returns** | the id naming that row |

### stmt_index

```mach
pub fun stmt_index(id: StmtId) u32
```

the statement table index an id names

| Parameter | |
| :--- | :--- |
| `id` | the statement id |
| **returns** | its table index, all ones for the nil id |

### stmt_same

```mach
pub fun stmt_same(left: StmtId, right: StmtId) bool
```

whether two statement ids name the same node

| Parameter | |
| :--- | :--- |
| `left` | first id |
| `right` | second id |
| **returns** | true when the ids are equal |

### stmt_is_nil

```mach
pub fun stmt_is_nil(id: StmtId) bool
```

whether a statement id is the nil id

| Parameter | |
| :--- | :--- |
| `id` | the statement id |
| **returns** | true when it is STMT_NIL |

## DeclId

```mach
pub def DeclId: u32
```

a declaration node

| Name | Value | |
| :--- | :--- | :--- |
| `DECL_NIL` | `0xFFFFFFFF` |  |

### decl

```mach
pub fun decl(index: u32) DeclId
```

the declaration id of a table index

| Parameter | |
| :--- | :--- |
| `index` | index into the declaration table |
| **returns** | the id naming that row |

### decl_index

```mach
pub fun decl_index(id: DeclId) u32
```

the declaration table index an id names

| Parameter | |
| :--- | :--- |
| `id` | the declaration id |
| **returns** | its table index, all ones for the nil id |

### decl_same

```mach
pub fun decl_same(left: DeclId, right: DeclId) bool
```

whether two declaration ids name the same node

| Parameter | |
| :--- | :--- |
| `left` | first id |
| `right` | second id |
| **returns** | true when the ids are equal |

### decl_is_nil

```mach
pub fun decl_is_nil(id: DeclId) bool
```

whether a declaration id is the nil id

| Parameter | |
| :--- | :--- |
| `id` | the declaration id |
| **returns** | true when it is DECL_NIL |

## TypeId

```mach
pub def TypeId: u32
```

a type-syntax node, distinct from the semantic type.TypeId

| Name | Value | |
| :--- | :--- | :--- |
| `TYPE_NIL` | `0xFFFFFFFF` |  |

### type

```mach
pub fun type(index: u32) TypeId
```

the type-syntax id of a table index

| Parameter | |
| :--- | :--- |
| `index` | index into the type-syntax table |
| **returns** | the id naming that row |

### type_index

```mach
pub fun type_index(id: TypeId) u32
```

the type-syntax table index an id names

| Parameter | |
| :--- | :--- |
| `id` | the type-syntax id |
| **returns** | its table index, all ones for the nil id |

### type_is_nil

```mach
pub fun type_is_nil(id: TypeId) bool
```

whether a type-syntax id is the nil id

| Parameter | |
| :--- | :--- |
| `id` | the type-syntax id |
| **returns** | true when it is TYPE_NIL |

## ModuleNodeId

```mach
pub def ModuleNodeId: u32
```

a module node

| Name | Value | |
| :--- | :--- | :--- |
| `MODULE_NODE_NIL` | `0xFFFFFFFF` |  |

### module_node

```mach
pub fun module_node(index: u32) ModuleNodeId
```

the module node id of a table index

| Parameter | |
| :--- | :--- |
| `index` | index into the module node table |
| **returns** | the id naming that row |

### module_node_index

```mach
pub fun module_node_index(id: ModuleNodeId) u32
```

the module node table index an id names

| Parameter | |
| :--- | :--- |
| `id` | the module node id |
| **returns** | its table index, all ones for the nil id |

### module_node_is_nil

```mach
pub fun module_node_is_nil(id: ModuleNodeId) bool
```

whether a module node id is the nil id

| Parameter | |
| :--- | :--- |
| `id` | the module node id |
| **returns** | true when it is MODULE_NODE_NIL |

