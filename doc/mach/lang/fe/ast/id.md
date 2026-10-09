# mach.lang.fe.ast.id

## rec ExprId

```mach
pub rec ExprId;
```

an expression node

## rec StmtId

```mach
pub rec StmtId;
```

a statement node

## rec DeclId

```mach
pub rec DeclId;
```

a declaration node

## rec TypeId

```mach
pub rec TypeId;
```

a type-syntax node, distinct from the semantic type.TypeId

## rec ModuleNodeId

```mach
pub rec ModuleNodeId;
```

a module node

## val EXPR_NIL

```mach
pub val EXPR_NIL:        ExprId       = ExprId;
```

## val STMT_NIL

```mach
pub val STMT_NIL:        StmtId       = StmtId;
```

## val DECL_NIL

```mach
pub val DECL_NIL:        DeclId       = DeclId;
```

## val TYPE_NIL

```mach
pub val TYPE_NIL:        TypeId       = TypeId;
```

## val MODULE_NODE_NIL

```mach
pub val MODULE_NODE_NIL: ModuleNodeId = ModuleNodeId;
```

## fun expr

```mach
pub fun expr(index: u32) ExprId;
```

## fun expr_index

```mach
pub fun expr_index(id: ExprId) u32;
```

## fun expr_same

```mach
pub fun expr_same(left: ExprId, right: ExprId) bool;
```

## fun expr_is_nil

```mach
pub fun expr_is_nil(id: ExprId) bool;
```

## fun stmt

```mach
pub fun stmt(index: u32) StmtId;
```

## fun stmt_index

```mach
pub fun stmt_index(id: StmtId) u32;
```

## fun stmt_same

```mach
pub fun stmt_same(left: StmtId, right: StmtId) bool;
```

## fun stmt_is_nil

```mach
pub fun stmt_is_nil(id: StmtId) bool;
```

## fun decl

```mach
pub fun decl(index: u32) DeclId;
```

## fun decl_index

```mach
pub fun decl_index(id: DeclId) u32;
```

## fun decl_same

```mach
pub fun decl_same(left: DeclId, right: DeclId) bool;
```

## fun decl_is_nil

```mach
pub fun decl_is_nil(id: DeclId) bool;
```

## fun type

```mach
pub fun type(index: u32) TypeId;
```

## fun type_index

```mach
pub fun type_index(id: TypeId) u32;
```

## fun type_is_nil

```mach
pub fun type_is_nil(id: TypeId) bool;
```

## fun module_node

```mach
pub fun module_node(index: u32) ModuleNodeId;
```

## fun module_node_index

```mach
pub fun module_node_index(id: ModuleNodeId) u32;
```

## fun module_node_is_nil

```mach
pub fun module_node_is_nil(id: ModuleNodeId) bool;
```

