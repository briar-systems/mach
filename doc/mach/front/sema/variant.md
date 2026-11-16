[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **variant**

# mach.front.sema.variant

the generic unions an annotation instantiates must have variants agreeing on secrecy
under their arguments, which no check of the generic declaration can answer

<sub>source: [`src/front/sema/variant.mach`](../../../../src/front/sema/variant.mach) · imported by 2 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `base_source` is [`mach.base.source`](../../base/source.md)</sub>

| Functions | |
| :--- | :--- |
| [`walk_begin`](#walk_begin) | opens the walk the checks of one scope's annotations share, which stays closed when the module holds no secret, so its checks have nothing to find |
| [`walk_end`](#walk_end) · [`check_annotation_uni_secrecy`](#check_annotation_uni_secrecy) | |
| [`check_type`](#check_type) | the check of `tid`, an annotation's type at `span`, under the scope's own substitution |

---

## Functions

### walk_begin

```mach
pub fun walk_begin(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk)
```

opens the walk the checks of one scope's annotations share, which stays closed when the
module holds no secret, so its checks have nothing to find

### walk_end

```mach
pub fun walk_end(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk)
```

### check_annotation_uni_secrecy

```mach
pub fun check_annotation_uni_secrecy(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk, ast_tid: ast_id.TypeId)
```

### check_type

```mach
pub fun check_type(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk, tid: type_kind.TypeId, span: base_source.Span)
```

the check of `tid`, an annotation's type at `span`, under the scope's own substitution

