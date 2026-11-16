[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **inferred**

# mach.front.sema.inferred

the lengths `[_]T` array types take from the array literals that fix them

a pass over a module's syntax before any type resolves: it gives a literal's own `[_]T`, and the
annotation of a `val` or `var` one initializes, the literal's element count. a `[_]` left unset
is refused where it resolves.

Source: [`src/front/sema/inferred.mach`](../../../../src/front/sema/inferred.mach)

Uses types from [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`record_all`](#record_all) | the one owner of "the length of an array literal" is its element count, which the `array.literal_length` check also reads |

---

## Functions

### record_all

```mach
pub fun record_all(sc: *mach.front.sema.context.SemaContext)
```

the one owner of "the length of an array literal" is its element count, which the
`array.literal_length` check also reads. this pass hands that count to the `[_]T` type nodes
a literal fixes, before any type resolves: the literal's own type spelling, and the annotation
of a `val` or `var` initialized by one. a `[_]` left unset is refused where it resolves

