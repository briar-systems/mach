[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **lint**

# mach.front.sema.lint

the lints over a typed module: uses of deprecated declarations, and testing
declarations reached from code outside a test

Source: [`src/front/sema/lint.mach`](../../../../src/front/sema/lint.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`warn_deprecated_uses`](#warn_deprecated_uses) | warn at each use of a deprecated declaration, where the build shows the module's warnings, since warning is all it does |
| [`check_testing_uses`](#check_testing_uses) | a reference to a `#[testing]` declaration is legal only inside a test body or another `#[testing]` declaration: every resolved name and type use is checked against the source ranges of those declarations, and an unmarked `fwd` may not re-export one |

---

## Functions

### warn_deprecated_uses

```mach
pub fun warn_deprecated_uses(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

warn at each use of a deprecated declaration, where the build shows the module's warnings, since
warning is all it does

### check_testing_uses

```mach
pub fun check_testing_uses(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

a reference to a `#[testing]` declaration is legal only inside a test body or another
`#[testing]` declaration: every resolved name and type use is checked against the source
ranges of those declarations, and an unmarked `fwd` may not re-export one

