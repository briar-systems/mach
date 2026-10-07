# mach.lang.fe.sema.literal

the checks over a typed module's literals: float precision, defaulted literals and shifts

## fun check_float_literals

```mach
pub fun check_float_literals(sc: *sema_context.SemaContext);
```

the float literal rule at each literal's final type, once every body has been inferred

## fun check_default_literals

```mach
pub fun check_default_literals(sc: *sema_context.SemaContext) err[fail.Fail];
```

a literal its context did not type took the i64 default, where the range rule has
not yet run: each outermost expression of untyped literals is checked at its final
type once every body has been inferred. one its context typed was checked when it
was coerced, and fits

## fun check_literal_shifts

```mach
pub fun check_literal_shifts(sc: *sema_context.SemaContext);
```

a shift of an untyped literal is typed by its context, so its count is checked
against that type's width once every body has been inferred

## fun check_instance_float_literals

```mach
pub fun check_instance_float_literals(sc: *sema_context.SemaContext, generic: *type.TypeId);
```

the rule for the literals an instance retypes: those its generic body left at a type
parameter. the rest were checked with their module

