[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **op**

# mach.front.sema.op

an `#[intrinsic]` function names an instruction of an owner's operation catalog: mirl's
capability op catalog of the target holds masc's rows of each set. a declaration for the
selected target is read against that catalog where a call to it is lowered, and mirl's
verifier holds the instruction to its row; one for any other target is inert

Source: [`src/front/sema/op.mach`](../../../../src/front/sema/op.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`validate_op`](#validate_op) | an instruction has no body of its own: each call is the instruction |
| [`target_is_selected`](#target_is_selected) | whether an `#[intrinsic]` naming the target `tgt` is an instruction of the selected one |

---

## Functions

### validate_op

```mach
pub fun validate_op(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

an instruction has no body of its own: each call is the instruction

### target_is_selected

```mach
pub fun target_is_selected(sc: *mach.front.sema.context.SemaContext, tgt: str) bool
```

whether an `#[intrinsic]` naming the target `tgt` is an instruction of the selected one

