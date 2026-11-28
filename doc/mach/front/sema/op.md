[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **op**

# mach.front.sema.op

an `#[intrinsic(owner, set, name)]` function names what a call of it becomes. an owner that
is an architecture names an instruction of that target's operation catalog, mirl's
capability catalog of the target, which holds masc's rows of each set: a declaration for
the selected architecture is read against that catalog where a call to it is lowered, and
mirl's verifier holds the instruction to its row, while one for any other architecture is an
ordinary function, its body what that target runs. the reserved owner `mirl` names one of
mirl's own operations of the set `ir`, on every target: the declaration has no body, takes
no type, `$` or pack parameter, and names an operation OPERATIONS admits, and lowering holds
its signature to the operation's row

Source: [`src/front/sema/op.mach`](../../../../src/front/sema/op.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.sema.context`](context.md), [`mach.front.sema.interface`](interface.md).

| Functions | |
| :--- | :--- |
| [`target_is_selected`](#target_is_selected) | whether the architecture name `tgt` names the selected architecture, as an `#[intrinsic]` or a `#[handle]` names it |
| [`intrinsic_of`](#intrinsic_of) | what the `#[intrinsic]` of the function `d` makes a call of it for the build, none where it has none or `check` refuses it |
| [`check`](#check) | the function `d` refused where its `#[intrinsic]` names one of mirl's operations and the declaration cannot be that operation: a set other than `ir`, a name `ir` holds no operation an `intrinsic` may name of, a body, a type parameter, a `$` parameter or a pack |

---

## Functions

### target_is_selected

```mach
pub fun target_is_selected(sc: *mach.front.sema.context.SemaContext, tgt: str) bool
```

whether the architecture name `tgt` names the selected architecture, as an `#[intrinsic]` or a `#[handle]` names it

### intrinsic_of

```mach
pub fun intrinsic_of(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl) mach.front.sema.interface.Intrinsic
```

what the `#[intrinsic]` of the function `d` makes a call of it for the build, none where it
has none or `check` refuses it

### check

```mach
pub fun check(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

the function `d` refused where its `#[intrinsic]` names one of mirl's operations and the
declaration cannot be that operation: a set other than `ir`, a name `ir` holds no operation
an `intrinsic` may name of, a body, a type parameter, a `$` parameter or a pack

