# mach.lang.fe.sema.inferred

## fun record_all

```mach
pub fun record_all(sc: *sema_context.SemaContext);
```

the one owner of "the length of an array literal" is its element count, which the
`array.literal_length` check also reads. this pass hands that count to the `[_]T` type nodes
a literal fixes, before any type resolves: the literal's own type spelling, and the annotation
of a `val` or `var` initialized by one. a `[_]` left unset is refused where it resolves

