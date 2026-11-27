[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **infer**

# mach.front.sema.infer

typing expressions: each expression's type, the checks on its operands, and the instances
its calls ask for

Source: [`src/front/sema/infer.mach`](../../../../src/front/sema/infer.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`infer_decl`](#infer_decl) | the type the declaration `did` declares: a function's signature, a nominal type, the type a `def` names or mints, or a binding's declared or initializer type |
| [`infer_expr`](#infer_expr) | types the expression `eid` and everything under it, records the type, and reports what it finds to `sc` |
| [`is_field_type_operand`](#is_field_type_operand) | whether `eid` is the `.type` member of a field or case descriptor, as a type comparison reads one |
| [`field_seq_owner`](#field_seq_owner) | the record a `$fields(T)` sequence enumerates, its field table made ready |
| [`case_seq_owner`](#case_seq_owner) | `$cases(T)` enumerates the cases of one tag, `^` or not, since naming a type's cases reads no value; a generic parameter defers to its instantiation |
| [`type_is_generic_param`](#type_is_generic_param) | whether `ty` is a generic type parameter |

---

## Functions

### infer_decl

```mach
pub fun infer_decl(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId) mach.type.kind.TypeId
```

the type the declaration `did` declares: a function's signature, a nominal type, the type a
`def` names or mints, or a binding's declared or initializer type

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `did` | the declaration |
| **returns** | the type, TYPE_NIL for a declaration that declares none, or the error type when the declaration is out of range or refused |

### infer_expr

```mach
pub fun infer_expr(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) mach.type.kind.TypeId
```

types the expression `eid` and everything under it, records the type, and reports what
it finds to `sc`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `eid` | the expression |
| **returns** | its type, the error type for a nil or refused expression |

### is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

whether `eid` is the `.type` member of a field or case descriptor, as a type comparison reads one

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `eid` | the expression asked about |
| **returns** | true when its object evaluates to a descriptor, or the failure of reading the expression |

### field_seq_owner

```mach
pub fun field_seq_owner(sc: *mach.front.sema.context.SemaContext, seq_eid: mach.front.ast.id.ExprId, span: mach.base.source.Span) mach.type.kind.TypeId
```

the record a `$fields(T)` sequence enumerates, its field table made ready

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `seq_eid` | the `$fields` call |
| `span` | where a refusal is reported |
| **returns** | the record, a generic parameter left to its instantiation, or the error type when the argument is missing, not a record, or its table cannot be made |

### case_seq_owner

```mach
pub fun case_seq_owner(sc: *mach.front.sema.context.SemaContext, seq_eid: mach.front.ast.id.ExprId, span: mach.base.source.Span) mach.type.kind.TypeId
```

`$cases(T)` enumerates the cases of one tag, `^` or not, since naming a type's cases reads no
value; a generic parameter defers to its instantiation

### type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *mach.front.sema.context.SemaContext, ty: mach.type.kind.TypeId) bool
```

whether `ty` is a generic type parameter

| Parameter | |
| :--- | :--- |
| `sc` | the sema context whose type table is read |
| `ty` | the type asked about |
| **returns** | true for a type parameter |

