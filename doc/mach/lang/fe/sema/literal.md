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
not yet run: each outermost expression of untyped literals still at that default is
checked once every body has been inferred. one its context typed was checked when it
was coerced, and fits or was refused, so only an expression typed i64 is asked about.
a refused one leaves its literals at the default, so they are marked inner all the same

## fun check_literal_shifts

```mach
pub fun check_literal_shifts(sc: *sema_context.SemaContext);
```

a shift of an untyped literal is typed by its context, so its count is checked
against that type's width once every body has been inferred

## fun check_instance_float_literals

```mach
pub fun check_instance_float_literals(sc: *sema_context.SemaContext, generic: *type_kind.TypeId);
```

the rule for the literals an instance retypes: those its generic body left at a type
parameter. the rest were checked with their module

