[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **infer**

# mach.front.sema.infer

typing expressions: each expression's type, the checks on its operands, and the instances
its calls ask for

Source: [`src/front/sema/infer.mach`](../../../../src/front/sema/infer.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`infer_decl`](#infer_decl), [`infer_expr`](#infer_expr), [`is_field_type_operand`](#is_field_type_operand) | |
| [`timed_of`](#timed_of) | the constant-time mode `#[constant_time]` gives declaration `d` of syntax `a`, read under the decorator scope `c` |
| [`field_seq_owner`](#field_seq_owner) | |
| [`case_seq_owner`](#case_seq_owner) | `$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer secrecy protects the active case, and a generic parameter defers to its instantiation |
| [`type_is_generic_param`](#type_is_generic_param) | |

---

## Functions

### infer_decl

```mach
pub fun infer_decl(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId) mach.type.kind.TypeId
```

### infer_expr

```mach
pub fun infer_expr(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) mach.type.kind.TypeId
```

### is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

### timed_of

```mach
pub fun timed_of(sc: *mach.front.sema.context.SemaContext, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl) u8
```

the constant-time mode `#[constant_time]` gives declaration `d` of syntax `a`, read under the decorator scope `c`

### field_seq_owner

```mach
pub fun field_seq_owner(sc: *mach.front.sema.context.SemaContext, seq_eid: mach.front.ast.id.ExprId, span: mach.base.source.Span) mach.type.kind.TypeId
```

### case_seq_owner

```mach
pub fun case_seq_owner(sc: *mach.front.sema.context.SemaContext, seq_eid: mach.front.ast.id.ExprId, span: mach.base.source.Span) mach.type.kind.TypeId
```

`$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer
secrecy protects the active case, and a generic parameter defers to its instantiation

### type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *mach.front.sema.context.SemaContext, ty: mach.type.kind.TypeId) bool
```

