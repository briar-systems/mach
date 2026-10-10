# mach.lang.fe.sema.variant

the generic unions an annotation instantiates must have variants agreeing on secrecy
under their arguments, which no check of the generic declaration can answer

## fun walk_open

```mach
pub fun walk_open(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk);
```

opens the walk the checks of one scope's annotations share, which stays closed when the
module holds no secret, so its checks have nothing to find

## fun walk_close

```mach
pub fun walk_close(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk);
```

## fun check_annotation_uni_secrecy

```mach
pub fun check_annotation_uni_secrecy(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk, ast_tid: ast_id.TypeId);
```

## fun check_type

```mach
pub fun check_type(sc: *sema_context.SemaContext, w: *sema_context.SecrecyWalk, tid: type.TypeId, span: lang_source.Span);
```

the check of `tid`, an annotation's type at `span`, under the scope's own substitution

