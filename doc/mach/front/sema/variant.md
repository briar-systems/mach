[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **variant**

# mach.front.sema.variant

the generic unions an annotation instantiates must have variants agreeing on secrecy
under their arguments, which no check of the generic declaration can answer

Source: [`src/front/sema/variant.mach`](../../../../src/front/sema/variant.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`walk_begin`](#walk_begin) | opens the walk the checks of one scope's annotations share, which stays closed when the module holds no secret, so its checks have nothing to find |
| [`walk_end`](#walk_end) | closes the shared walk walk_begin opened, a closed walk is left as it is |
| [`check_annotation_uni_secrecy`](#check_annotation_uni_secrecy) | check that the generic unions the annotation `ast_tid`'s type reaches agree on secrecy, reporting at the annotation, nothing is checked when the walk is closed or the annotation has no resolved type |
| [`check_type`](#check_type) | the check of `tid`, an annotation's type at `span`, under the scope's own substitution |

---

## Functions

### walk_begin

```mach
pub fun walk_begin(sc: *mach.front.sema.context.SemaContext, w: *mach.front.sema.context.SecrecyWalk)
```

opens the walk the checks of one scope's annotations share, which stays closed when the
module holds no secret, so its checks have nothing to find

### walk_end

```mach
pub fun walk_end(sc: *mach.front.sema.context.SemaContext, w: *mach.front.sema.context.SecrecyWalk)
```

closes the shared walk walk_begin opened, a closed walk is left as it is

| Parameter | |
| :--- | :--- |
| `sc` | semantic context whose type store holds the walk's scans |
| `w` | the walk to close |

### check_annotation_uni_secrecy

```mach
pub fun check_annotation_uni_secrecy(sc: *mach.front.sema.context.SemaContext, w: *mach.front.sema.context.SecrecyWalk, ast_tid: mach.front.ast.id.TypeId)
```

check that the generic unions the annotation `ast_tid`'s type reaches agree on secrecy,
reporting at the annotation, nothing is checked when the walk is closed or the
annotation has no resolved type

| Parameter | |
| :--- | :--- |
| `sc` | semantic context of the scope being checked |
| `w` | the shared walk opened by walk_begin, or nil |
| `ast_tid` | the annotation's syntax type |

### check_type

```mach
pub fun check_type(sc: *mach.front.sema.context.SemaContext, w: *mach.front.sema.context.SecrecyWalk, tid: mach.type.kind.TypeId, span: mach.base.source.Span)
```

the check of `tid`, an annotation's type at `span`, under the scope's own substitution

