[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **id**

# mach.front.ast.id

the ids of the syntax nodes in one tree

each id is the index of its node in the table for its kind, with all ones as its nil. the ids
are transparent `def` aliases of `u32`, so the compiler does not keep one kind of id from
standing where another, or a bare integer, is meant.

<sub>source: [`src/front/ast/id.mach`](../../../../src/front/ast/id.mach) · imported by 60 modules</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ExprId`](#exprid) | `def` | an expression node |
| [`StmtId`](#stmtid) | `def` | a statement node |
| [`DeclId`](#declid) | `def` | a declaration node |
| [`TypeId`](#typeid) | `def` | a type-syntax node, distinct from the semantic type.TypeId |
| [`ModuleNodeId`](#modulenodeid) | `def` | a module node |

| On `ExprId` | |
| :--- | :--- |
| [`EXPR_NIL`](#exprid) · [`expr`](#expr) · [`expr_index`](#expr_index) · [`expr_same`](#expr_same) · [`expr_is_nil`](#expr_is_nil) | |

| On `StmtId` | |
| :--- | :--- |
| [`STMT_NIL`](#stmtid) · [`stmt`](#stmt) · [`stmt_index`](#stmt_index) · [`stmt_same`](#stmt_same) · [`stmt_is_nil`](#stmt_is_nil) | |

| On `DeclId` | |
| :--- | :--- |
| [`DECL_NIL`](#declid) · [`decl`](#decl) · [`decl_index`](#decl_index) · [`decl_same`](#decl_same) · [`decl_is_nil`](#decl_is_nil) | |

| On `TypeId` | |
| :--- | :--- |
| [`TYPE_NIL`](#typeid) · [`type`](#type) · [`type_index`](#type_index) · [`type_is_nil`](#type_is_nil) | |

| On `ModuleNodeId` | |
| :--- | :--- |
| [`MODULE_NODE_NIL`](#modulenodeid) · [`module_node`](#module_node) · [`module_node_index`](#module_node_index) · [`module_node_is_nil`](#module_node_is_nil) | |

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

### expr_index

```mach
pub fun expr_index(id: ExprId) u32
```

### expr_same

```mach
pub fun expr_same(left: ExprId, right: ExprId) bool
```

### expr_is_nil

```mach
pub fun expr_is_nil(id: ExprId) bool
```

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

### stmt_index

```mach
pub fun stmt_index(id: StmtId) u32
```

### stmt_same

```mach
pub fun stmt_same(left: StmtId, right: StmtId) bool
```

### stmt_is_nil

```mach
pub fun stmt_is_nil(id: StmtId) bool
```

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

### decl_index

```mach
pub fun decl_index(id: DeclId) u32
```

### decl_same

```mach
pub fun decl_same(left: DeclId, right: DeclId) bool
```

### decl_is_nil

```mach
pub fun decl_is_nil(id: DeclId) bool
```

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

### type_index

```mach
pub fun type_index(id: TypeId) u32
```

### type_is_nil

```mach
pub fun type_is_nil(id: TypeId) bool
```

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

### module_node_index

```mach
pub fun module_node_index(id: ModuleNodeId) u32
```

### module_node_is_nil

```mach
pub fun module_node_is_nil(id: ModuleNodeId) bool
```

