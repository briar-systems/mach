[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **literal**

# mach.front.sema.literal

the checks over a typed module's literals: float precision, defaulted literals and shifts

Source: [`src/front/sema/literal.mach`](../../../../src/front/sema/literal.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`check_float_literals`](#check_float_literals) | the float literal rule at each literal's final type, once every body has been inferred |
| [`check_default_literals`](#check_default_literals) | a literal its context did not type took the i64 default, where the range rule has not yet run: each outermost expression of untyped literals still at that default is checked once every body has been inferred |
| [`check_literal_shifts`](#check_literal_shifts) | a shift of an untyped literal is typed by its context, so its count is checked against that type's width once every body has been inferred |
| [`check_instance_float_literals`](#check_instance_float_literals) | the rule for the literals an instance retypes: those its generic body left at a type parameter |

---

## Functions

### check_float_literals

```mach
pub fun check_float_literals(sc: *mach.front.sema.context.SemaContext)
```

the float literal rule at each literal's final type, once every body has been inferred

### check_default_literals

```mach
pub fun check_default_literals(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

a literal its context did not type took the i64 default, where the range rule has
not yet run: each outermost expression of untyped literals still at that default is
checked once every body has been inferred. one its context typed was checked when it
was coerced, and fits or was refused, so only an expression typed i64 is asked about.
a refused one leaves its literals at the default, so they are marked inner all the same

### check_literal_shifts

```mach
pub fun check_literal_shifts(sc: *mach.front.sema.context.SemaContext)
```

a shift of an untyped literal is typed by its context, so its count is checked
against that type's width once every body has been inferred

### check_instance_float_literals

```mach
pub fun check_instance_float_literals(sc: *mach.front.sema.context.SemaContext, generic: *mach.type.kind.TypeId)
```

the rule for the literals an instance retypes: those its generic body left at a type
parameter. the rest were checked with their module

